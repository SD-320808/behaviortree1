# First Behavior Tree
## Installing BehaviorTree 

First install the behaviortree.cpp from https://github.com/BehaviorTree/BehaviorTree.CPP
Clone the repository and build it
```
git clone https://github.com/BehaviorTree/BehaviorTree.CPP.git
mkdir build; cd build
cmake ..
make
sudo make install
```
## Running the behavior tree
Clone this repository and build it
```
git clone https://github.com/SD-320808/behaviortree1.git
cd catkin_ws
source devel/setup.bash
catkin_make
```

Now run the tree
```
rosrun hello_BT BT
```
