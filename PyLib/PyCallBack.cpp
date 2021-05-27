/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/



#include "PythonInputHandler.h"
#ifdef HAS_PYTHON
#include <QEventLoop>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include "PythonToolsPanel.h"
#include "PyCallBack.h"

CPythonInputHandler* PyGetInput(int type)
{
    auto wnd = PRV::getMainWindow();
    auto inputHandler = wnd->GetPythonToolsPanel()->getInputHandler();

    QMetaObject::invokeMethod(inputHandler, "getInput", Q_ARG(int, type));

    QEventLoop loop;
    QObject::connect(inputHandler, &CPythonInputHandler::inputReady, &loop, &QEventLoop::quit);
    loop.exec();

    return inputHandler;
}

std::string PyGetString()
{
    return PyGetInput(CPythonInputHandler::STRING)->getString();
}

int PyGetInt()
{
    return PyGetInput(CPythonInputHandler::INT)->getInt();
}

#else
CPythonInputHandler* PyGetInput(int type) {return nullptr;}
std::string PyGetString() {return "";}
int PyGetInt(){return 0;}
#endif