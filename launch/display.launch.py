from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, Shutdown
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, PythonExpression, Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():

    # --------------------------------------------------
    # Package path
    # --------------------------------------------------
    desc_pkg = get_package_share_directory('mecanumrover_description')

    # --------------------------------------------------
    # Launch arguments
    # --------------------------------------------------
    rover = LaunchConfiguration('rover', default='mecanum3')
    gui   = LaunchConfiguration('gui',   default='true')

    # --------------------------------------------------
    # Select xacro and rviz by rover type
    # --------------------------------------------------
    model_file = PythonExpression([
        '"', desc_pkg, '/urdf/" + (',
        '"mecanum3.xacro" if "', rover, '" == "mecanum3" else ',
        '"g40a_lb.xacro" if "', rover, '" == "g40a_lb" else ',
        '"g120a.xacro")'
    ])

    rviz_file = PythonExpression([
        '"', desc_pkg, '/rviz/" + (',
        '"mecanum3.rviz" if "', rover, '" == "mecanum3" else ',
        '"g40a_lb.rviz" if "', rover, '" == "g40a_lb" else ',
        '"g120a.rviz")'
    ])

    # --------------------------------------------------
    # Robot description
    # --------------------------------------------------
    robot_description = ParameterValue(
        Command(['xacro ', model_file, ' use_ros2_control:=false']),
        value_type=str
    )

    # --------------------------------------------------
    # Nodes
    # --------------------------------------------------
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{
            'robot_description': robot_description
        }]
    )

    joint_state_publisher = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        condition=UnlessCondition(gui)
    )

    joint_state_publisher_gui = Node(
        prefix='setsid',
        package='mecanumrover_description',
        executable='joint_state_gui.py',
        condition=IfCondition(gui)
    )

    rviz = Node(
        package='mecanumrover_description',
        executable='mecanumrover_rviz',
        name='rviz2',
        exec_name='rviz2',
        arguments=['-d', rviz_file],
        output='screen',
        on_exit=Shutdown(reason='RViz closed')
    )

    # --------------------------------------------------
    # Launch description
    # --------------------------------------------------
    return LaunchDescription([
        DeclareLaunchArgument(
            'rover',
            default_value='mecanum3',
            description='Rover type: mecanum3 | g120a | g40a_lb'
        ),
        DeclareLaunchArgument(
            'gui',
            default_value='true',
            description='Use joint_state_publisher_gui'
        ),
        robot_state_publisher,
        joint_state_publisher,
        joint_state_publisher_gui,
        rviz
    ])
