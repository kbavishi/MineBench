%% This file were automatically generated by SWIG's MatLab module
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% [result] = CreateUsingClusters(varargin)
%%
%% C++ prototype: pnl::CBKInfEngine *pnl::CBKInfEngine::Create(pnl::CDynamicGraphicalModel const *pGrModelIn,pnl::intVecVector &clusters)
%%

function [result] = CreateUsingClusters(varargin)

[result] = feval('pnl_full', 'CBKInfEngine_CreateUsingClusters_wrap', varargin{:});
result = CBKInfEngine('%%@#DefaultCtor', result);

return
