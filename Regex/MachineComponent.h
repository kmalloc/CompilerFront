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
    State_Ref  = 0x80,
};

struct MachineState
{
    MachineState(int num, StateType t)
        :no(num), type(t)
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        ,unit_type(0)
        ,parentUnit(-1)
#endif
    {
    }

    StateType GetType() const { return type; }
    void SetNormType() { type = (StateType)((type & ~(State_Accept | State_Start)) | State_Norm); }
    void SetType(StateType t) { type = t; }
    void AppendType(StateType t) { type = (StateType)(type | t); }
    void ClearType(StateType t) { type = (StateType)(type & (~t)); }

    bool IsHeadState() const { return type & State_Head; }
    bool IsTailState() const { return type & State_Tail; }

    int no;
    StateType type;

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    short unit_type;
    int   parentUnit;
    short UnitStart() const { return unit_type & 0x00ff; }
    short UnitEnd() const { return (unit_type & 0xff00) >> 8; }
    void SetStartUnit(int num = 1) { unit_type += num; }
    void SetEndUnit(int num = 1) { unit_type += (num << 8); }

    bool IsRefState() const { return type & State_Ref; }
    int  GetParentUnit() const { return parentUnit; }
    void SetParentUnit(int unit) { parentUnit = unit; }
#endif

};

struct MachineEdge
{
    short ch; // -1 means epsilon
    int to;
    int from; // state number
};

#endif

