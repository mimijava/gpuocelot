/*!
	\file ReconvergenceMechanism.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Nov 15, 2010
	\brief extracts the reconvergence mechanism from CooperativeThreadArray
*/

#ifndef OCELOT_EXECUTIVE_RECONVERGENCEMECHANISM_H_INCLUDED
#define OCELOT_EXECUTIVE_RECONVERGENCEMECHANISM_H_INCLUDED

// C++ includes
#include <deque>

// Ocelot includes
#include <ocelot/executive/interface/CTAContext.h>
#include <ocelot/ir/interface/PTXOperand.h>
#include <ocelot/ir/interface/Kernel.h>
#include <ocelot/ir/interface/Texture.h>
#include <ocelot/trace/interface/TraceEvent.h>

////////////////////////////////////////////////////////////////////////////////

namespace executive {

class EmulatedKernel;
class CooperativeThreadArray;

/*!
	\brief base class for abstract reconvergence mechanism within emulator
*/
class ReconvergenceMechanism {
public:
	typedef std::vector<CTAContext> RuntimeStack;
		
	enum Type {
		Reconverge_IPDOM,
		Reconverge_Barrier,
		Reconverge_TFGen6,
		Reconverge_TFSortedStack,
		Reconverge_unknown
	};
		
public:
	ReconvergenceMechanism(const executive::EmulatedKernel *kernel,
		CooperativeThreadArray *cta);
	ReconvergenceMechanism(CooperativeThreadArray *cta);
	
public:

	//! \brief initializes the reconvergence mechanism
	virtual void initialize();

	//! \brief updates the predicate mask of the active context
	// before instructions execute
	virtual void evalPredicate(executive::CTAContext &context) = 0;
	
	/*! 
		\brief implements branch instruction and updates CTA state
		\return true on divergent branch
	*/
	virtual bool eval_Bra(executive::CTAContext &context, 
		const ir::PTXInstruction &instr, 
		const boost::dynamic_bitset<> & branch, 
		const boost::dynamic_bitset<> & fallthrough) = 0;

	/*! 
		\brief implements a barrier instruction
	*/
	virtual void eval_Bar(executive::CTAContext &context,
		const ir::PTXInstruction &instr) = 0;
	
	/*!
		\brief implements reconverge instruction
	*/
	virtual void eval_Reconverge(executive::CTAContext &context,
		const ir::PTXInstruction &instr) = 0;
	
	/*!
		\brief implements exit instruction
	*/
	virtual void eval_Exit(executive::CTAContext &context,
		const ir::PTXInstruction &instr) = 0;

	/*! 
		\brief updates the active context to the next instruction
	*/
	virtual bool nextInstruction(executive::CTAContext &context,
		const ir::PTXInstruction &instr) = 0;
	
	//! \brief gets the active context
	executive::CTAContext& getContext();
	
	//! \brief gets the stack size
	size_t stackSize() const;
	
	//! \brief gets the reconvergence mechanism type
	Type getType() const { return type; }

	//! \brief gets a string-representation of the type
	static std::string toString(Type type);

public:

	//! \brief dynamic type information for convergence mechanism
	Type type;

	//! \brief emulated kernel instance
	const EmulatedKernel *kernel;
	
	//! \brief executing CTA
	CooperativeThreadArray *cta;

	//! \brief context stack
	RuntimeStack runtimeStack;
};

//
//
//

class ReconvergenceIPDOM: public ReconvergenceMechanism {
public:

	ReconvergenceIPDOM(const executive::EmulatedKernel *kernel,
		CooperativeThreadArray *cta);
	
	void evalPredicate(executive::CTAContext &context);
	bool eval_Bra(executive::CTAContext &context, 
		const ir::PTXInstruction &instr, 
		const boost::dynamic_bitset<> & branch, 
		const boost::dynamic_bitset<> & fallthrough);
	void eval_Bar(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Reconverge(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Exit(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	bool nextInstruction(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
};

class ReconvergenceBarrier: public ReconvergenceMechanism {
public:

	ReconvergenceBarrier(const executive::EmulatedKernel *kernel,
		CooperativeThreadArray *cta);
	
	void evalPredicate(executive::CTAContext &context);
	bool eval_Bra(executive::CTAContext &context, 
		const ir::PTXInstruction &instr, 
		const boost::dynamic_bitset<> & branch, 
		const boost::dynamic_bitset<> & fallthrough);
	void eval_Bar(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Reconverge(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Exit(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	bool nextInstruction(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
};


class ReconvergenceTFGen6: public ReconvergenceMechanism {
public:
	typedef std::vector <int> ThreadIdVector;
	
public:
	ReconvergenceTFGen6(const executive::EmulatedKernel *kernel,
		CooperativeThreadArray *cta);

	void initialize();
	void evalPredicate(executive::CTAContext &context);
	bool eval_Bra(executive::CTAContext &context, 
		const ir::PTXInstruction &instr, 
		const boost::dynamic_bitset<> & branch, 
		const boost::dynamic_bitset<> & fallthrough);
	void eval_Bar(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Reconverge(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Exit(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	bool nextInstruction(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	
public:

	//! \brief program counters for each thread
	ThreadIdVector threadPCs;
};

class ReconvergenceTFSortedStack: public ReconvergenceMechanism {
public:
	ReconvergenceTFSortedStack(const executive::EmulatedKernel *kernel,
		CooperativeThreadArray *cta);

	void evalPredicate(executive::CTAContext &context);
	bool eval_Bra(executive::CTAContext &context, 
		const ir::PTXInstruction &instr, 
		const boost::dynamic_bitset<> & branch, 
		const boost::dynamic_bitset<> & fallthrough);
	void eval_Bar(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Reconverge(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	void eval_Exit(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
	bool nextInstruction(executive::CTAContext &context,
		const ir::PTXInstruction &instr);
};
}

////////////////////////////////////////////////////////////////////////////////

#endif

