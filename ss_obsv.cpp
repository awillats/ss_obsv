/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
 * Weill Cornell Medical College
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a template implementation file for a user module derived from
 * DefaultGUIModel with a custom GUI.
 */

#include "ss_obsv.h"
#include <iostream>
#include <main_window.h>

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new SsObsv();
}

static DefaultGUIModel::variable_t vars[] = {
	{
		"y","output", DefaultGUIModel::OUTPUT,
	},
  {
    "x1", "Tooltip description", DefaultGUIModel::OUTPUT,
  },
  {
    "x2", "Tooltip description", DefaultGUIModel::OUTPUT,
  },

	{
		"ustim","input", DefaultGUIModel::INPUT,
	},
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

SsObsv::SsObsv(void)
  : DefaultGUIModel("State-space observer (Kalman Filter)", ::vars, ::num_vars)
{
  setWhatsThis("<p><b>SsObsv:</b><br>QWhatsThis description.</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  customizeGUI();
  initParameters();
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

SsObsv::~SsObsv(void)
{
}


void
SsObsv::loadSys(void)
{	
	std::ifstream myfile;
	myfile.open("../ss_ctrl/params/obsv_params.txt");

	//std::cout<<"load works here"<<"\n";
	// numA;
	//halp::simpleFun();
	pullParamLine(myfile); //gets nx

	std::vector<double> numA = pullParamLine(myfile); 	
	Eigen::Map<Eigen::Matrix2d> tA(numA.data(),A.rows(),A.cols());
	A = tA;
	
	std::vector<double> numB = pullParamLine(myfile); 	
	Eigen::Map<Eigen::Vector2d> tB(numB.data(),B.rows(),1);
	B = tB;

	std::vector<double> numC = pullParamLine(myfile); 	
	Eigen::Map<Eigen::RowVector2d> tC(numC.data(),1,C.cols());
	C = tC;

	//For some silly reason, can't load D this way
	std::vector<double> numD = pullParamLine(myfile); 	
	D = numD[0];
	
	//K,Q,R



	myfile.close();
}

void 
SsObsv::printSys(void)
{
  std::cout <<"Here is the matrix A:\n" << A << "\n";
  std::cout <<"Here is the matrix B:\n" << B << "\n";
  std::cout <<"Here is the matrix C:\n" << C << "\n";
  std::cout <<"Here is the matrix D:\n" << D << "\n";
}

void SsObsv::resetSys(void)
{
	x << 0,0;
	y = 0;
	u = 0;
}


void SsObsv::stepKF(double uin)
{
	//prediction
	Eigen::Vector2d xpred;
	Eigen::Matrix2d Ppred;
	Eigen::Vector2d xup;
	Eigen::Matrix2d Pup;

	xpred = A*x+B*u;
	Ppred = A*P*A.transpose()+Q;

	//update

	Eigen::Matrix2d IKC = Eigen::Matrix2d::Identity() - K*C; 

	xup = xpred + K*(y-C*xpred);
	Ppred = IKC*Ppred*IKC.transpose() + K*R*K.transpose();

	//update gain
	Eigen::Matrix2d CPCR = C*Ppred*C.transpose() + R;
	K = Ppred*C.transpose() *CPCR.inverse();
}



void
SsObsv::execute(void)
{
  stepKF(input(0));
  return;
}

void
SsObsv::initParameters(void)
{
	loadSys();
	resetSys();
	printSys();
}

void
SsObsv::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    case MODIFY:
      some_parameter = getParameter("GUI label").toDouble();
      break;

    case UNPAUSE:
      break;

    case PAUSE:
      break;

    case PERIOD:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    default:
      break;
  }
}

void
SsObsv::customizeGUI(void)
{
  QGridLayout* customlayout = DefaultGUIModel::getLayout();

  QGroupBox* button_group = new QGroupBox;

  QPushButton* abutton = new QPushButton("Load System");
  QPushButton* bbutton = new QPushButton("Reset System");
  QHBoxLayout* button_layout = new QHBoxLayout;
  button_group->setLayout(button_layout);
  button_layout->addWidget(abutton);
  button_layout->addWidget(bbutton);
  QObject::connect(abutton, SIGNAL(clicked()), this, SLOT(aBttn_event()));
  QObject::connect(bbutton, SIGNAL(clicked()), this, SLOT(bBttn_event()));

  customlayout->addWidget(button_group, 0, 0);
  setLayout(customlayout);
}

// functions designated as Qt slots are implemented as regular C++ functions
void
SsObsv::aBttn_event(void)
{
	loadSys();
	printSys();
}

void
SsObsv::bBttn_event(void)
{
	resetSys();
}
