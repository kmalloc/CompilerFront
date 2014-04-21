#ifndef MACHINE_COMPONENT_H_
#define MACHINE_COMPONENT_H_

enum StateType
{
    State_None, // not a state
    State_Start,
    State_Accept,
    State_Epsilon,
    State_Norm,
    State_Dead,
    // TODO
    State_BackRef,
};

#define STATE_TRAN_MAX (256)
#define STATE_EPSILON (STATE_TRAN_MAX - 1)

struct MachineState
{
    MachineState(int num, StateType t)
        :no(num), type(t)
    {
    }

    void SetType(StateType t) { type = t; }

    int no;
    StateType type;
};

struct MachineEdge
{
    short ch; // -1 means epsilon
    int to;
    int from; // state number
};

#endif

