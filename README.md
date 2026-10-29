# メカナムローバーの3Dモデルのパッケージ

![](images/mecanum3_description.png)

## 準備
Ubuntu 24.04にROS 2 JazzyとGazebo Harmonicをインストールしてから、ワークスペースに本パッケージと`vs_rover_options_description`を配置してビルドします。
以下は新しいワークスペースを作成する例です。

```bash
source /opt/ros/jazzy/setup.bash
mkdir -p ~/rover_ws/src
cd ~/rover_ws/src
git clone --branch jazzy https://github.com/vstoneofficial/mecanumrover_description.git mecanumrover_description
git clone --branch jazzy https://github.com/vstoneofficial/vs_rover_options_description.git vs_rover_options_description
cd ..
rosdep install --from-paths src --ignore-src --rosdistro jazzy -y
colcon build --symlink-install --packages-select vs_rover_options_description mecanumrover_description
source install/setup.bash
```

1. 対応するオプション
   - LRF TG30
   - バンパー（mecanum3、g120a）

2. 使用する機種のXacro内で、オプションのコメントを解除してください。変更後は再ビルドしてください。

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


## RViz上の可視化

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


## Gazebo Harmonicでのシミュレーション

`rover:=`で、RVizと同じ機種を指定します。

```bash
ros2 launch mecanumrover_description gazebo.launch.py rover:=mecanum3
```

別の端末でワークスペースを読み込み、`rover_twist`に速度指令を送ります。

```bash
source ~/rover_ws/install/setup.bash
ros2 topic pub --once /rover_twist geometry_msgs/msg/Twist '{linear: {x: 0.2}}'
```

停止するときは速度を0にします。終了はGazeboを起動した端末でCtrl+Cを押してください。

```bash
ros2 topic pub --once /rover_twist geometry_msgs/msg/Twist '{}'
```
