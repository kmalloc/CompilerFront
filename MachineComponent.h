#ifndef MACHINE_COMPONENT_H_
#define MACHINE_COMPONENT_H_

enum StateType
{
    State_None = 0, // not a state
    State_Start = 0x1,
    State_Accept = 0x2,
    State_Epsilon = 0x4,
    State_Norm = 0x8,
    State_Head = 0x10,
    State_Tail = 0x20,
    State_Dead = 0x40,
    // TODO
    State_BackRef = 0x80,
};

#define STATE_TRAN_MAX (256)
#define STATE_EPSILON (STATE_TRAN_MAX - 1)

struct MachineState
{
    MachineState(int num, StateType t)
        :no(num), type(t)
    {
    }

    void SetNormType() { type = (StateType)((type & ~(State_Accept | State_Start)) | State_Norm); }
    void SetType(StateType t) { type = t; }
    void AppendType(StateType t) { type = (StateType)(type | t); }

    bool IsHeadState() const { return type & State_Head; }
    bool IsTailState() const { return type & State_Tail; }

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

