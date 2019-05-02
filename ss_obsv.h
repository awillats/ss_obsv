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
 * This is a template header file for a user modules derived from
 * DefaultGUIModel with a custom GUI.
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../../../module_help/StAC_rtxi/dataFuns.h"//for pullParamLine

//#include "help.h"

#include <default_gui_model.h>

#include "../../../module_help/eigen/Eigen/Dense"

// plds
#include <dynCtrlEst>


class SsObsv : public DefaultGUIModel
{

  Q_OBJECT

public:
  SsObsv(void);
  virtual ~SsObsv(void);

  void execute(void);
  void createGUI(DefaultGUIModel::variable_t*, int);
  void customizeGUI(void);

protected:
  virtual void update(DefaultGUIModel::update_flags_t);

private:
  double some_parameter;
  double some_state;
  double period;


int switch_idx;

/*
Eigen::Tensor As;
Eigen::Tensor Bs;
Eigen::Tensor Cs;
Eigen::Tensor Ds;
*/
	Eigen::Matrix2d A;
	Eigen::Vector2d B;
	Eigen::RowVector2d C;
	float D;

	Eigen::Matrix2d A_;
	Eigen::Vector2d B_;
	Eigen::RowVector2d C_;
	float D_;

	Eigen::Matrix2d A2;
	Eigen::Vector2d B2;
	Eigen::RowVector2d C2;
	float D2;


  	Eigen::RowVector2d K_obsv;
  	Eigen::RowVector2d K_obsv_;
  	Eigen::RowVector2d K_obsv2;

	Eigen::Vector2d x;
	float y;

	float u;

  void switchSys(int);

  void loadSys(void);
  void resetSys(void);
  void printSys(void);
  void stepPlant(double,double);
  void initParameters();

private slots:
  // these are custom functions that can also be connected to events
  // through the Qt API. they must be implemented in plugin_template.cpp

  void aBttn_event(void);
  void bBttn_event(void);
};
