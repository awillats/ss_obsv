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

#include <default_gui_model.h>
#include "../../../module_help/eigen/Eigen/Dense"
#include "../../../module_help/StAC_rtxi/dataFuns.h"//for pullParamLine

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

  	Eigen::Matrix2d A;
	Eigen::Vector2d B;
	Eigen::RowVector2d C;
	float D;

	Eigen::Vector2d x;
	float y;
	float u;

	Eigen::RowVector2d K;
	Eigen::Matrix2d P;
	Eigen::Matrix2d Q;
	double R;




  void loadSys();
  void printSys();
  void resetSys();
  void stepKF(double uin);

  void initParameters();

private slots:
  // these are custom functions that can also be connected to events
  // through the Qt API. they must be implemented in plugin_template.cpp

  void aBttn_event(void);
  void bBttn_event(void);
};
