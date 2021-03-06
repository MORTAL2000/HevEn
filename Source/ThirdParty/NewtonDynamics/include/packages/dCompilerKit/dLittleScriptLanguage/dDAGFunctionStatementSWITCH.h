/* Copyright (c) <2009> <Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#ifndef __dDAGFunctionStatementSWITCH_H__
#define __dDAGFunctionStatementSWITCH_H__

#include "dDAG.h"
#include "dDAGFunctionStatementFlow.h"

class dDAGExpressionNode;
class dDAGFunctionStatementCase;

class dDAGFunctionStatementSWITCH: public dDAGFunctionStatementFlow
{
	public:
	dDAGFunctionStatementSWITCH(dList<dDAG*>& allNodes, dDAGExpressionNode* const expression, dDAGFunctionStatementCase* const caseList);
	~dDAGFunctionStatementSWITCH();

	virtual void CompileCIL(dCIL& cil);
	virtual void ConnectParent(dDAG* const parent);

	virtual dDAGFunctionStatement* const GetPostFixStatement() const;
	dDAGRtti(dDAGFunctionStatementFlow);

	dDAGExpressionNode* m_expression;
	dList<dDAGFunctionStatementCase*> m_caseList;
};


#endif