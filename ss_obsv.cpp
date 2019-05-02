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
	//plds::stdVec u_vec = inputVector(2);
	//double u_fromvec = u_vec[0];

	double u_total = u_pre; //+u_fromvec
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


void
SsObsv::loadSys(void)
{	

	std::string homepath = getenv("HOME");
	std::ifstream myfile;
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_ctrl/params/plant_params.txt");

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
	//std::cout <<"ww"<< *numD.begin()<<"ww\n";
	D = numD[0];
	//D = (float) numD.at(0);
	
	myfile.close();


	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_obsv/params/obsv_params.txt");
	pullParamLine(myfile); //gets nx
	std::vector<double> vK = pullParamLine(myfile); 	
	Eigen::Map<Eigen::RowVector2d> tK(vK.data(),1,K_obsv.cols());//,1,K.cols());
	K_obsv = tK;
	myfile.close();



/*
	//look on stackoverflow @ initialize eigenvector with stdvector
	float data[] = {1,2,3,4};
	Eigen::Map<Eigen::Vector3f> v1(data);
	std::vector<float> data2= {1,2,3,4};
	Eigen::Vector3f v2(data2.data());
	std::cout<<v2<<"?\n";
	//Eigen::Matrix2f zz;
*/
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

/*
	A << 0.9990, 0.0095,
		-0.1903, 0.9039;
	B << 0,
		 0.0095;
	C << 1,0;
	D=0;
*/

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

