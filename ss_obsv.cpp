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

using namespace adam;

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

	{"y_det","output", DefaultGUIModel::OUTPUT,},
	{"y_kf","output", DefaultGUIModel::OUTPUT,},
	{"y_skf","output", DefaultGUIModel::OUTPUT,},

	{ "X_out", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{ "X_kf", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{ "X_switch", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{"debug","normP",DefaultGUIModel::OUTPUT},



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

/*
void SsObsv::stepObsv(double uin, double ymeas)
{
	u = uin;
	x = A*x + B*u - K_obsv.transpose()*(y-ymeas);//transpose?
	y = C*x;
	sys.x=x;
	sys.y=y;
}
*/
void
SsObsv::execute(void)
{
	double u_pre = input(0);
	double u_total = u_pre;

	double ymeas = input(1);

	switch_idx = input(2);
	skf.switchSys(switch_idx);


	obsv.predict(u_total, ymeas);
	kalman.predict(u_total, ymeas);
	skf.predict(u_total,ymeas);

	y = obsv.y;
	
	output(0) = y;
	output(1) = kalman.y;
	output(2) = skf.y;

	outputVector(3) = arma::conv_to<stdVec>::from(x);
	outputVector(4) = arma::conv_to<stdVec>::from(kalman.x);
	outputVector(5) = arma::conv_to<stdVec>::from(skf.x);

	output(6) = arma::norm(kalman.P);
	
  return;
}

/*
void SsObsv::switchSys(int idx)
{
	x = sys.x;//snapshot current system state
	//at the moment x is held in ss_plant and operated on
	sys = ((idx==0) ? sys1 : sys2);

	A = sys.A;
	B = sys.B;
	C = sys.C;
	D = sys.D;
	sys.x = x; //make sure new system has up to date state
}

void
SsObsv::loadGains(void)
{	
	//read in system params from file
	std::string homepath = getenv("HOME");
	std::ifstream myfile;

	//read in observer gains from file
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_obsv/params/obsv_params.txt");
	pullParamLine(myfile); //gets nx
	K_obsv = stdVec2EigenRV(pullParamLine(myfile), K_obsv.cols());
	myfile.close();

	K_obsv_=K_obsv;
	K_obsv2=K_obsv/switch_scale;
}
*/
void SsObsv::resetAllSys(void)
{
	sys.resetSys();
	sys1.resetSys();
	sys2.resetSys();


	obsv.resetSys();
	obsv.x.randn();

	kalman.resetSys();
	kalman.x.randn();

	skf.resetSys();
	skf.x.randn();

}




void
SsObsv::initParameters(void)
{
   switch_scale=1.4;

	sys = lds_adam();
	sys.initSys();

	sys1 = sys;
	sys2 = sys;
	sys2.B = sys2.B*switch_scale;

	//loadGains();


	
	obsv = lds_obsv();
	kalman = glds_obsv();
	skf = s_glds_obsv();
	//kalman.isUpdating=-1;

/*
	obsv.predict(4,5);
	kalman.predict(4,5);

	obsv.predict(0,5);
	kalman.predict(0,5);
std::cout<<">"<<obsv.y<<"_"<<kalman.y<<"<";

	obsv.predict(0,5);
	kalman.predict(0,5);

	obsv.predict(0,5);
	kalman.predict(0,5);
	std::cout<<">"<<obsv.y<<"_"<<kalman.y<<"<";
*/
/*
	skf.predict(10,100);
skf.predict(1,10);
skf.predict(2,3);
skf.predict(4,0);
	std::cout<<"\n\nSKIF TEST:"<<skf.switchScale;
	std::cout<<"\nST2"<<skf.P;
*/

}

void
SsObsv::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    case MODIFY:
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
	initParameters();
}
void
SsObsv::bBttn_event(void)
{
	resetAllSys();
}


void SsObsv::zBttn_event(bool tog)
{
	kalman.toggleUpdating();
	//initParameters();

	if (tog)
	{
		obsv.K = 0*obsv.K;
	}
	else
	{
		lds_obsv newObsv = lds_obsv();
		obsv.K = newObsv.K;
	}

	
}

