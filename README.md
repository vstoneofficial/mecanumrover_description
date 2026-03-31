# mecanumrover_description

`mecanumrover_description` は、メカナムローバー系列の URDF/Xacro、Gazebo 設定、RViz 設定、各種補助スクリプトをまとめたパッケージです。

対応ロボット:
- `mecanum3`
- `g120a`
- `g40a_lb`

## 構成

- `urdf/`
  - 各ロボットの Xacro、Gazebo 用定義、ros2_control 用定義
- `meshes/`
  - 3D モデル
- `launch/`
  - RViz 表示用 launch
- `rviz/`
  - RViz 設定ファイル
- `params/`
  - ros2_control のコントローラ設定
- `scripts/`
  - 補助ノードや設定ファイル

## 必要なもの

このパッケージ単体で完結しません。少なくとも以下を用意してください。
- ROS 2 Humble
- `xacro`
- `robot_state_publisher`
- `rviz2`
- `vs_rover_options_description` を使う場合はそのパッケージ

インストール例:

```bash
sudo apt install ros-humble-xacro ros-humble-robot-state-publisher ros-humble-rviz2
```

## インストール

ワークスペースに入れてビルドします。

```bash
mkdir -p "$HOME/ros2_ws/src"
cd "$HOME/ros2_ws/src"
git clone https://github.com/vstoneofficial/mecanumrover_description.git
cd "$HOME/ros2_ws"
colcon build --symlink-install
source install/setup.bash
```

## 使い方

### RViz で表示

ロボットモデルを表示します。`rover:=` で機種を切り替えます。

```bash
ros2 launch mecanumrover_description display.launch.py rover:=mecanum3
```

`rover:=` で指定できる値:
- `mecanum3`
- `g120a`
- `g40a_lb`

### 既定のモデルを確認

`mecanum3` を表示する場合:

```bash
ros2 launch mecanumrover_description display.launch.py rover:=mecanum3
```

`g120a` を表示する場合:

```bash
ros2 launch mecanumrover_description display.launch.py rover:=g120a
```

`g40a_lb` を表示する場合:

```bash
ros2 launch mecanumrover_description display.launch.py rover:=g40a_lb
```

## 注意

- 3D モデルや Xacro の参照パスは、公開先のワークスペース配置に依存しないよう相対的に扱ってください。
- `display.launch.py` は RViz 表示用です。Gazebo 連携や制御起動は別の bringup / gazebo パッケージ側で行います。

## ライセンス

このパッケージは Apache 2.0 ライセンスの下で提供されています。詳細は [LICENSE](./LICENSE) を参照してください。
