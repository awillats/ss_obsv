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

/*
	{"y_det","output", DefaultGUIModel::OUTPUT,}, //0
	{"y_kf","output", DefaultGUIModel::OUTPUT,}, //1
	{"y_skf","output", DefaultGUIModel::OUTPUT,}, //2
	{"y_ppf","output", DefaultGUIModel::OUTPUT,}, //3
	{"expy_ppf","output", DefaultGUIModel::OUTPUT,}, //4
*/
	{"y_sppf","linear", DefaultGUIModel::OUTPUT,}, //5

/*
	{ "X_out", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, }, //6
	{ "X_kf", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, }, //7
	{ "X_switch", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, }, //8
	{ "X_ppf", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, }, //9
*/
	{ "X_sppf", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, }, //10

//	{ "debug","normP", DefaultGUIModel::OUTPUT}, //11



	{"ustim","input", DefaultGUIModel::INPUT,}, //0

	{"y_meas","measured y", DefaultGUIModel::INPUT,}, //1
	{"spike_meas","spikes in", DefaultGUIModel::INPUT,}, //2

	{"q","state_index", DefaultGUIModel::INPUT | DefaultGUIModel::INTEGER}, //3

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

void
SsObsv::execute(void)
{
	//convert these to data_t?
	double u_pre = input(0);
	double u_total = u_pre;
	double ymeas = input(1);
	double spike_meas = input(2);

	switch_idx = input(3);
	//skf.switchSys(switch_idx);
	sppf.switchSys(switch_idx);
/*
	obsv.predict(u_total, ymeas);
	kalman.predict(u_total, ymeas);
	skf.predict(u_total,ymeas);

	ppf.predict(u_total, spike_meas);
*/
	sppf.predict(u_total, spike_meas);

	y = sppf.y;//obsv.y;
	
	output(0) = y;
/*
	output(1) = kalman.y;
	output(2) = skf.y;
	output(3) = ppf.y;
	output(4) = ppf.y_nl;

	output(5) = sppf.y;
*/

	outputVector(1) = arma::conv_to<stdVec>::from(sppf.x);

/*
	outputVector(6) = arma::conv_to<stdVec>::from(x);
	outputVector(7) = arma::conv_to<stdVec>::from(kalman.x);
	outputVector(8) = arma::conv_to<stdVec>::from(skf.x);

	outputVector(9) = arma::conv_to<stdVec>::from(ppf.x);
	outputVector(10) = arma::conv_to<stdVec>::from(sppf.x);

	output(11) = arma::norm(kalman.P);
*/
	
  return;
}

void SsObsv::resetAllSys(void)
{
/*
	sys.resetSys();
	sys1.resetSys();
	sys2.resetSys();


	obsv.resetSys();
	obsv.x.randn();

	skf.resetSys();
	skf.x.randn();

	kalman.resetSys();
	kalman.x.randn();

	ppf.resetSys();
	ppf.x.randn();
*/
	sppf.resetSys();
	sppf.x.randn();
}


void
SsObsv::initParameters(void)
{
   switch_scale=1.4;
/*
	sys = lds_adam();
	sys.initSys();

	sys1 = sys;
	sys2 = sys;
	sys2.B = sys2.B*switch_scale;

	//loadGains();

	obsv = lds_obsv();	
	skf = s_glds_obsv();
	kalman = glds_obsv();

	ppf = plds_obsv();
*/
	sppf = s_plds_obsv();


//	std::cout<<"PPF, dt:"<<ppf.dt<<", nl_d:"<<ppf.nl_d;
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
/*
	kalman.toggleUpdating();
		//std::cout<<"\nskf report pre:"<<skf.isUpdating;
	skf.toggleUpdating();
		//std::cout<<"skf report post:"<<skf.isUpdating<<"\n";
	//initParameters();
	ppf.toggleUpdating();
*/
	sppf.toggleUpdating();

/*
	if (tog)
	{
		obsv.K = 0*obsv.K;
	}
	else
	{
		lds_obsv newObsv = lds_obsv();
		obsv.K = newObsv.K;
	}
*/
	
}

