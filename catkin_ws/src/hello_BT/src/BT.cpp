#include<iostream>
#include<unistd.h>
#include"behaviortree_cpp_v3/action_node.h"
#include"behaviortree_cpp_v3/basic_types.h"
#include"behaviortree_cpp_v3/behavior_tree.h"
#include"behaviortree_cpp_v3/blackboard.h"
#include"behaviortree_cpp_v3/bt_factory.h"
#include"behaviortree_cpp_v3/bt_parser.h"
#include"behaviortree_cpp_v3/condition_node.h"
#include"behaviortree_cpp_v3/control_node.h"
#include"behaviortree_cpp_v3/decorator_node.h"
#include"behaviortree_cpp_v3/exceptions.h"
#include"behaviortree_cpp_v3/leaf_node.h"
#include"behaviortree_cpp_v3/tree_node.h"
#include"behaviortree_cpp_v3/xml_parsing.h"
#include"behaviortree_cpp_v3/actions/always_failure_node.h"
#include"behaviortree_cpp_v3/actions/always_success_node.h"
#include"behaviortree_cpp_v3/actions/set_blackboard_node.h"
#include"behaviortree_cpp_v3/controls/fallback_node.h"
#include"behaviortree_cpp_v3/controls/parallel_node.h"
#include"behaviortree_cpp_v3/controls/reactive_fallback.h"
#include"behaviortree_cpp_v3/controls/reactive_sequence.h"
#include"behaviortree_cpp_v3/controls/sequence_node.h"
#include"behaviortree_cpp_v3/controls/sequence_star_node.h"
#include"behaviortree_cpp_v3/controls/switch_node.h"
#include"behaviortree_cpp_v3/decorators/blackboard_precondition.h"
#include"behaviortree_cpp_v3/decorators/force_failure_node.h"
#include"behaviortree_cpp_v3/decorators/force_success_node.h"
#include"behaviortree_cpp_v3/decorators/inverter_node.h"
#include"behaviortree_cpp_v3/decorators/repeat_node.h"
#include"behaviortree_cpp_v3/decorators/retry_node.h"
#include"behaviortree_cpp_v3/decorators/subtree_node.h"
#include"behaviortree_cpp_v3/decorators/timeout_node.h"
#include"behaviortree_cpp_v3/decorators/timer_queue.h"
#include"behaviortree_cpp_v3/utils/any.hpp"
#include"behaviortree_cpp_v3/utils/simple_string.hpp"
#include"behaviortree_cpp_v3/utils/string_view.hpp"
static const char* xml_text = R"( 
 <root>
     <BehaviorTree>
        <Sequence>
            <BatteryOK/>
            <SaySomething   message="mission started..." />
            <MoveBase       goal="1;2;3"/>
            <SaySomething   message="mission completed!" />
        </Sequence>
     </BehaviorTree>
 </root>
)";
using namespace BT;
struct Pose2D
{
    double x, y, theta;
};
namespace BT
{
    template <> inline Pose2D convertFromString(StringView str)
    {
        // The next line should be removed...
        printf("Converting string: \"%s\"\n", str.data() );

        // We expect real numbers separated by semicolons
        auto parts = splitString(str, ';');
        if (parts.size() != 3)
        {
            throw RuntimeError("invalid input)");
        }
        else{
            Pose2D output;
            output.x     = convertFromString<double>(parts[0]);
            output.y     = convertFromString<double>(parts[1]);
            output.theta = convertFromString<double>(parts[2]);
            return output;
        }
    }
} 

class MoveBaseAction : public AsyncActionNode
{
  public:
    MoveBaseAction(const std::string& name, const NodeConfiguration& config)
      : AsyncActionNode(name, config)
    { }

    static PortsList providedPorts()
    {
        return{ InputPort<Pose2D>("goal") };
    }

    NodeStatus tick() override;

    // This overloaded method is used to stop the execution of this node.
    void halt() override
    {
        _halt_requested.store(true);
    }

  private:
    std::atomic_bool _halt_requested;
};

//-------------------------

NodeStatus MoveBaseAction::tick()
{
    Pose2D goal;
    if ( !getInput<Pose2D>("goal", goal))
    {
        throw RuntimeError("missing required input [goal]");
    }

    printf("[ MoveBase: STARTED ]. goal: x=%.f y=%.1f theta=%.2f\n", 
           goal.x, goal.y, goal.theta);

    _halt_requested.store(false);
    int count = 0;

    // Pretend that "computing" takes 250 milliseconds.
    // It is up to you to check periodicall _halt_requested and interrupt
    // this tick() if it is true.
    while (!_halt_requested && count++ < 25)
    {
        sleep(1);
    }

    std::cout << "[ MoveBase: FINISHED ]" << std::endl;
    return _halt_requested ? NodeStatus::FAILURE : NodeStatus::SUCCESS;
}

class SaySomething : public SyncActionNode
{
  public:
    // If your Node has ports, you must use this constructor signature 
    SaySomething(const std::string& name, const NodeConfiguration& config)
      : SyncActionNode(name, config)
    { }

    // It is mandatory to define this static method.
    static PortsList providedPorts()
    {
        // This action has a single input port called "message"
        // Any port must have a name. The type is optional.
        return { InputPort<std::string>("message") };
    }

    // As usual, you must override the virtual function tick()
    NodeStatus tick() override
    {
        Optional<std::string> msg = getInput<std::string>("message");
        // Check if optional is valid. If not, throw its error
        if (!msg)
        {
            throw BT::RuntimeError("missing required input [message]: ", 
                                   msg.error() );
        }

        // use the method value() to extract the valid message.
        std::cout << "Robot says: " << msg.value() << std::endl;
        return NodeStatus::SUCCESS;
    }
};

BT::NodeStatus CheckBattery()
{
    std::cout << "[ Battery: OK ]" << std::endl;
    return BT::NodeStatus::SUCCESS;
}

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerSimpleCondition("BatteryOK", std::bind(CheckBattery));
    factory.registerNodeType<MoveBaseAction>("MoveBase");
    factory.registerNodeType<SaySomething>("SaySomething");

    auto tree = factory.createTreeFromText(xml_text);

    NodeStatus status;

    std::cout << "\n--- 1st executeTick() ---" << std::endl;
    status = tree.tickRoot();

    sleep(15);
    std::cout << "\n--- 2nd executeTick() ---" << std::endl;
    status = tree.tickRoot();

    sleep(15);
    std::cout << "\n--- 3rd executeTick() ---" << std::endl;
    status = tree.tickRoot();

    std::cout << std::endl;

    return 0;
}
