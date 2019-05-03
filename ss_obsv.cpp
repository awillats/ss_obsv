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
 * do gui elements last, just get outputs to screen
 * 
 */

#include "ss_obsv.h"
#include <main_window.h>

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new SsObsv();
}

static DefaultGUIModel::variable_t vars[] = {
  {
    "State-space plant", "Tooltip description",
    DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,
  },

	{
	    "y","output", DefaultGUIModel::OUTPUT,
	},
	{ "X_out", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
  {
    "x1", "Tooltip description", DefaultGUIModel::OUTPUT,
  },
  {
    "x2", "Tooltip description", DefaultGUIModel::OUTPUT,
  },


	{
		"ustim","input", DefaultGUIModel::INPUT,
	},
	{
		"y_meas","measured y", DefaultGUIModel::INPUT,
	},
	{"q","state_index", DefaultGUIModel::INPUT | DefaultGUIModel::INTEGER},

};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

SsObsv::SsObsv(void)
  : DefaultGUIModel("SsObsv with Custom GUI", ::vars, ::num_vars)
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


void SsObsv::stepPlant(double uin, double ymeas)
{
	u = uin;
	x = A*x + B*u - K_obsv.transpose()*(y-ymeas);//transpose?
	y = C*x;
}

void
SsObsv::execute(void)
{
	double u_pre = input(0);

	double u_total = u_pre;
	stepPlant(u_total, input(1));
	setState("x1",x(0));
	setState("x2",x(1));
	
	output(0) = y;

	std::vector<double>xstd(x.data(),x.data()+x.size());

	outputVector(1) = xstd;
	output(2) = x(0);
	output(3) = x(1);
	
  return;
}


void SsObsv::switchSys(int idx)
{
	if (idx==0)
	{
		A=A_;
		B=B_;
		C=C_;
		D=D_;
	}
	else
	{
		A=A2;
		B=B2;
		C=C2;
		D=D2;
	}
}

void
SsObsv::loadSys(void)
{	
	//read in system params from file
	std::string homepath = getenv("HOME");
	std::ifstream myfile;
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_ctrl/params/plant_params.txt");

	pullParamLine(myfile); //gets nx
		
	A = stdVec2EigenM(pullParamLine(myfile), A.rows(), A.cols());
	B = stdVec2EigenV(pullParamLine(myfile), B.rows());
	C = stdVec2EigenRV(pullParamLine(myfile), C.cols());

	std::vector<double> numD = pullParamLine(myfile); 	
	D = numD[0];
	myfile.close();

	//generate second switched system
	double switchScale = 1.4;
	A_=A;
	B_=B;
	C_=C;
	D_=D;

	A2=A;
	B2=B*switchScale;
	C2=C;
	D2=D;

	//read in observer gains from file
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_obsv/params/obsv_params.txt");
	pullParamLine(myfile); //gets nx
	K_obsv = stdVec2EigenRV(pullParamLine(myfile), K_obsv.cols());
	myfile.close();

	K_obsv_=K_obsv;
	K_obsv2=K_obsv/switchScale;
}

void SsObsv::printSys(void)
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


void
SsObsv::initParameters(void)
{
  some_parameter = 0;
  some_state = 0;

	K_obsv << 0.0,0.0;//hardcode
	loadSys();
	printSys();
	resetSys();
}

void
SsObsv::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      setParameter("GUI label", some_parameter);
      //setState("A State", some_state);
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

  QPushButton* abutton = new QPushButton("Load Matrices");
  QPushButton* bbutton = new QPushButton("Reset Sys");

  QPushButton* zbutton = new QPushButton("Set ctrl gain to 0");
  zbutton->setCheckable(true);

  QHBoxLayout* button_layout = new QHBoxLayout;
  button_group->setLayout(button_layout);
  button_layout->addWidget(abutton);
  button_layout->addWidget(bbutton);
  button_layout->addWidget(zbutton);
  QObject::connect(abutton, SIGNAL(clicked()), this, SLOT(aBttn_event()));
  QObject::connect(bbutton, SIGNAL(clicked()), this, SLOT(bBttn_event()));
  QObject::connect(zbutton, SIGNAL(toggled(bool)), this, SLOT(zBttn_event(bool)));

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


void SsObsv::zBttn_event(bool tog)
{
	loadSys();
	if (tog)
	{
		K_obsv << 0.0,0.0;//hardcode
		K_obsv_ = K_obsv;// << 0.0,0.0;//hardcode
		K_obsv2 = K_obsv;// << 0.0,0.0;//hardcode
	}
	printSys();
}

