/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLabel>
#include "DlgImageFilter.h"
#include "ClassDescriptor.h"


CDlgImageFilter::CDlgImageFilter()
{
    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(new QLabel("Choose an Image Filter:"));
    
    m_comboBox = new QComboBox;

	for (Class_Iterator it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
	{
        ClassDescriptor* pcd = *it;
		if (pcd->GetType() == CLASS_IMAGE_FILTER)
		{
            m_classDesc.push_back(pcd);
        
            m_comboBox->addItem(pcd->GetName());
        }
    }

    layout->addWidget(m_comboBox);

    QDialogButtonBox* bBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    layout->addWidget(bBox);

    setLayout(layout);

    connect(bBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ClassDescriptor* CDlgImageFilter::GetClassDescriptor()
{
    return m_classDesc[m_comboBox->currentIndex()];
}