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
%% [result] = CreatePriorSliceGrModel(varargin)
%%
%% C++ prototype: pnl::CStaticGraphicalModel *CreatePriorSliceGrModel(pnl::CDBN const *self)
%%

function [result] = CreatePriorSliceGrModel(varargin)

[result] = feval('pnl_full', 'CDBN_CreatePriorSliceGrModel_wrap', varargin{:});
result = CStaticGraphicalModel('%%@#DefaultCtor', result);

return
