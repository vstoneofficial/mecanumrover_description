// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

#include <QApplication>
#include <QSocketNotifier>
#include <QThread>

#include <rclcpp/rclcpp.hpp>
#include <rviz_common/logging.hpp>
#include <rviz_common/ros_integration/ros_client_abstraction_iface.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction.hpp>
#include <rviz_common/visualization_frame.hpp>
#include <rviz_common/visualization_manager.hpp>
#include <rviz_common/visualizer_app.hpp>

namespace
{
volatile std::sig_atomic_t shutdown_fd = -1;

void request_shutdown(int)
{
  const int saved_errno = errno;
  const char request = 1;
  if (shutdown_fd >= 0) {
    const auto written = ::write(shutdown_fd, &request, 1);
    (void)written;
  }
  errno = saved_errno;
}

class ShutdownPipe
{
public:
  ShutdownPipe()
  {
    if (::pipe2(fds_, O_NONBLOCK | O_CLOEXEC) != 0) {
      throw std::runtime_error(std::strerror(errno));
    }
    shutdown_fd = fds_[1];
    struct sigaction action {};
    action.sa_handler = request_shutdown;
    ::sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    if (::sigaction(SIGINT, &action, &previous_int_) != 0 ||
      ::sigaction(SIGTERM, &action, &previous_term_) != 0)
    {
      throw std::runtime_error("Cannot register viewer shutdown signals");
    }
  }

  ~ShutdownPipe()
  {
    ::sigaction(SIGINT, &previous_int_, nullptr);
    ::sigaction(SIGTERM, &previous_term_, nullptr);
    shutdown_fd = -1;
    ::close(fds_[0]);
    ::close(fds_[1]);
  }

  int reader() const {return fds_[0];}

private:
  int fds_[2];
  struct sigaction previous_int_ {}, previous_term_ {};
};

// Keep ROS alive until the Qt thread has stopped the RViz update timer.
class ViewerClient : public rviz_common::ros_integration::RosClientAbstractionIface
{
public:
  rviz_common::ros_integration::RosNodeAbstractionIface::WeakPtr init(
    int argc, char ** argv, const std::string & name, bool anonymous) override
  {
    if (anonymous) {
      throw std::invalid_argument("Anonymous RViz node names are unsupported");
    }
    rclcpp::init(argc, argv, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    node_ = std::make_shared<rviz_common::ros_integration::RosNodeAbstraction>(name);
    return node_;
  }

  bool ok() override {return rclcpp::ok() && node_ != nullptr;}
  void shutdown() override {rclcpp::shutdown();}

private:
  std::shared_ptr<rviz_common::ros_integration::RosNodeAbstraction> node_;
};
}  // namespace

int main(int argc, char ** argv)
{
  try {
    ShutdownPipe shutdown_pipe;
    auto qt_arguments = rclcpp::remove_ros_arguments(argc, argv);
    if (qEnvironmentVariable("XDG_SESSION_TYPE") == "wayland" &&
      !qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
      std::find(qt_arguments.begin(), qt_arguments.end(), "-platform") == qt_arguments.end())
    {
      qt_arguments.emplace_back("-platform");
      qt_arguments.emplace_back("xcb");
    }
    std::vector<char *> qt_argv;
    for (auto & argument : qt_arguments) {
      qt_argv.push_back(argument.data());
    }
    qt_argv.push_back(nullptr);
    int qt_argc = static_cast<int>(qt_arguments.size());
    QApplication application(qt_argc, qt_argv.data());
    const auto logger = rclcpp::get_logger("rviz2");
    rviz_common::set_logging_handlers(
      [logger](const std::string & msg, const std::string &, size_t) {
        RCLCPP_DEBUG(logger, "%s", msg.c_str());
      },
      [logger](const std::string & msg, const std::string &, size_t) {
        RCLCPP_INFO(logger, "%s", msg.c_str());
      },
      [logger](const std::string & msg, const std::string &, size_t) {
        RCLCPP_WARN(logger, "%s", msg.c_str());
      },
      [logger](const std::string & msg, const std::string &, size_t) {
        RCLCPP_ERROR(logger, "%s", msg.c_str());
      });
    rviz_common::VisualizerApp viewer(std::make_unique<ViewerClient>());
    viewer.setApp(&application);
    if (!viewer.init(argc, argv)) {
      return 1;
    }

    bool rendering_stopped = false;
    QSocketNotifier shutdown_notifier(shutdown_pipe.reader(), QSocketNotifier::Read);
    auto stop_rendering = [&]() {
        if (rendering_stopped) {
          return;
        }
        unsigned int stopped_managers = 0;
        for (auto * widget : QApplication::topLevelWidgets()) {
          auto * frame = qobject_cast<rviz_common::VisualizationFrame *>(widget);
          if (frame && frame->getManager()) {
            frame->getManager()->stopUpdate();
            ++stopped_managers;
          }
        }
        rendering_stopped = true;
        RCLCPP_INFO(logger, "RViz updates stopped: timers=%u, Qt thread=%s, ROS ok=%s",
          stopped_managers, QThread::currentThread() == application.thread() ? "true" : "false",
          rclcpp::ok() ? "true" : "false");
      };
    QObject::connect(
      &application, &QCoreApplication::aboutToQuit, &shutdown_notifier, stop_rendering);
    QObject::connect(&shutdown_notifier, &QSocketNotifier::activated, [&]() {
        shutdown_notifier.setEnabled(false);
        stop_rendering();
        QCoreApplication::exit(0);
      });
    return application.exec();
  } catch (const std::exception & error) {
    std::cerr << "mecanumrover RViz: " << error.what() << std::endl;
    return 1;
  }
}
