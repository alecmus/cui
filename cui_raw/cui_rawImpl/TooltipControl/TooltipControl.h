/*
** TooltipControl.h - tooltip control interface
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*/

#pragma once

#include "../cui_rawImpl.h"

void ShowToolTip(cui_rawImpl::ToolTipControl &tooltip);

void HideToolTip(cui_rawImpl::ToolTipControl &tooltip);
