#ifndef QTAPP_HPP
#define QTAPP_HPP

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QObject>
#include <QComboBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QGroupBox>
#include "state.hpp"


//A specialty combobox that changes a variable passed in by reference whenever the combobox is changed
class MyComboBox: public QComboBox
{
  Q_OBJECT
  public:
    MyComboBox(int& valToChange, bool& changedParam, QWidget *myParent=0 ) : QComboBox(myParent),index(valToChange),changed(changedParam),parent(myParent)
    {
      index=valToChange;
      connect(this,SIGNAL(currentIndexChanged(int)), this,
        SLOT(myIndexChanged(int)));
        //myParent.setValue(valToChange);

    }
    int getIndex(){ return index;}
    void changeIndex(int param)
    {
      myIndexChanged(param);
    }
  private slots:
    void myIndexChanged( int indexParam)
    {
      index=indexParam;
      changed=true;
      printf("index %i\n", index);

    }

 private:
   int& index;
   bool& changed;
   QWidget* parent;


};


class MySpinBox: public QSpinBox
{
  Q_OBJECT
  public:
    MySpinBox(int& inputValue, bool& changedParam, QWidget *myParent=0):QSpinBox(myParent),val(inputValue),changed(changedParam)
    {
      int temp=inputValue;
      connect(this,SIGNAL(valueChanged(int)),this,
        SLOT(myValueChanged(int)));
      setRange(10,100);
      setSuffix("%");
      setSingleStep(10);
      setValue(temp);
      //val=30;
    }
  private slots:
    void myValueChanged(int value)
    {
      val=value;
      changed=true;
    }
  private:
   int& val;
   bool& changed;

};

class TimeSpinBox: public QSpinBox
{
  Q_OBJECT
  public:
    TimeSpinBox(int& inputValue, bool& changedParam, QWidget *myParent=0):QSpinBox(myParent),val(inputValue),changed(changedParam)
    {
      connect(this,SIGNAL(valueChanged(int)),this,
        SLOT(myValueChanged(int)));

    }
  private slots:
    void myValueChanged(int value)
    {
      val=value;
      changed=true;
    }
  private:
   int& val;
   bool& changed;

};



class QTApp
{
  public:
    QTApp(int argc, char**argv, State & stateParam):state(stateParam)
    {

      QApplication app(argc, argv);

     QGroupBox horizontalGroupBox;
     QGroupBox verticalGroupBox1;
     QGroupBox verticalGroupBox2;
     QHBoxLayout hlayout;

     hlayout.addWidget(&verticalGroupBox1);
     hlayout.addWidget(&verticalGroupBox2);
     QVBoxLayout vlayout1;
     QVBoxLayout vlayout2;

     horizontalGroupBox.setLayout(&hlayout);

     verticalGroupBox1.setLayout(&vlayout1);
     verticalGroupBox2.setLayout(&vlayout2);


      skyType=0;
      renderRes=0;
      normalsOn=0;


      QWidget widg;
      QVBoxLayout layout;

      //QPushButton button("Select");
      //QObject::connect(&button, SIGNAL(clicked()), &app, SLOT(quit()));
      //button.resize(75,30);
      MyComboBox skyCombo(state.skyType,state.changed);
      skyCombo.addItem("Clear Sky");
      skyCombo.addItem("Clear turbid Sky");
      skyCombo.addItem("Intermediate Sky");
      skyCombo.addItem("Overcast Sky");
      MyComboBox renderResCombo(state.renderRes,state.changed);
      renderResCombo.addItem("Full res");
      renderResCombo.addItem("Patches");
      renderResCombo.addItem("Triangle");
      renderResCombo.addItem("Hybrid");
      renderResCombo.addItem("Do not use");//Tri moments...this needs to be debugged
      renderResCombo.addItem("No interpolation");

      MyComboBox normalCombo(state.debugMode,state.changed);
      normalCombo.addItem("Standard display");
      normalCombo.addItem("display normals");

      MyComboBox bounceCombo(state.bounceType,state.changed);
      bounceCombo.addItem("All");//0
      bounceCombo.addItem("direct sun");//1
      bounceCombo.addItem("indirect sun");//2
      bounceCombo.addItem("indirect");//3
      bounceCombo.addItem("direct sky");//4
      bounceCombo.addItem("indirect sky");//5
      bounceCombo.addItem("Sun only (photons)");//6


      MyComboBox viewpointCombo(state.viewpoint,state.changed);
      viewpointCombo.addItem("normal");
      viewpointCombo.addItem("eye");

      int tempScreen=state.screen;
      MyComboBox windowCombo(state.screen,state.changed);
      windowCombo.addItem("Off");
      windowCombo.addItem("On");


      MyComboBox directCombo(state.direct,state.changed);
      directCombo.addItem("Per Pixel");
      directCombo.addItem("Per Tri");

      MyComboBox indirectCombo(state.indirect,state.changed);
      indirectCombo.addItem("Per Tri");
      indirectCombo.addItem("Per Pixel");
      
      MyComboBox cameraCombo(state.cameraType,state.changed);
      cameraCombo.addItem("Standard");
      cameraCombo.addItem("Fisheye");


//    QObject::connect(&button, SIGNAL(clicked()), &app, SLOT(quit()));
      QLabel skyLabel;
      skyLabel.setText("Sky type");
      vlayout1.addWidget(&skyLabel);
      vlayout2.addWidget(&skyCombo);

      QLabel renderResLabel;
      renderResLabel.setText("Rendering resolution");
      vlayout1.addWidget(&renderResLabel);
      vlayout2.addWidget(&renderResCombo);
      //renderResCombo.setCurrentIndex(1);

      QLabel normalLabel;
      normalLabel.setText("Debug output");
      vlayout1.addWidget(&normalLabel);
      vlayout2.addWidget(&normalCombo);



      QLabel bounceLabel;
      bounceLabel.setText("Which bounces");
      vlayout1.addWidget(&bounceLabel);
      vlayout2.addWidget(&bounceCombo);


      QLabel viewpointLabel;
      viewpointLabel.setText("Viewpoint");
      vlayout1.addWidget(&viewpointLabel);
      vlayout2.addWidget(&viewpointCombo);

      QLabel windowLabel;
      windowLabel.setText("Screen");
      vlayout1.addWidget(&windowLabel);
      vlayout2.addWidget(&windowCombo);
      //windowCombo.setValue(state.screen);
      windowCombo.changeIndex(tempScreen);

      QLabel directLabel;
      directLabel.setText("Direct Light res");
      vlayout1.addWidget(&directLabel);
      vlayout2.addWidget(&directCombo);

      QLabel indirectLabel;
      indirectLabel.setText("Indirect Light res");
      vlayout1.addWidget(&indirectLabel);
      vlayout2.addWidget(&indirectCombo);

      QLabel cameraLabel;
      cameraLabel.setText("Camera Type");
      vlayout1.addWidget(&cameraLabel);
      vlayout2.addWidget(&cameraCombo);

      QLabel exposureLabel;
      exposureLabel.setText("Exposure %");
      vlayout1.addWidget(&exposureLabel);
      MySpinBox exposureBox(state.exposurePercent,state.changed);
      exposureBox.setValue(state.exposurePercent);
      vlayout2.addWidget(&exposureBox);


      QLabel monthLabel;
      monthLabel.setText("Month");
      vlayout1.addWidget(&monthLabel);
      TimeSpinBox monthBox(state.month,state.changed);
      monthBox.setValue(state.month);
      monthBox.setRange(1,12);

      vlayout2.addWidget(&monthBox);

      QLabel dayLabel;
      dayLabel.setText("Day");
      vlayout1.addWidget(&dayLabel);
      TimeSpinBox dayBox(state.day,state.changed);
      dayBox.setValue(state.day);
      dayBox.setRange(1,31);

      vlayout2.addWidget(&dayBox);

      QLabel hourLabel;
      hourLabel.setText("Hour");
      vlayout1.addWidget(&hourLabel);
      TimeSpinBox hourBox(state.hour,state.changed);
      hourBox.setRange(0,23);
      hourBox.setValue(state.hour);
      vlayout2.addWidget(&hourBox);

      QLabel minuteLabel;
      minuteLabel.setText("Minute");
      vlayout1.addWidget(&minuteLabel);
      TimeSpinBox minuteBox(state.minute,state.changed);
      minuteBox.setValue(state.minute);
      minuteBox.setRange(0,59);
      vlayout2.addWidget(&minuteBox);

      QLabel numBounceLabel;
      numBounceLabel.setText("# of bounces");
      vlayout1.addWidget(&numBounceLabel);
      TimeSpinBox numBounceBox(state.bounce,state.changed);
      numBounceBox.setRange(0,10);
      numBounceBox.setValue(10);
      vlayout2.addWidget(&numBounceBox);

      widg.setLayout(&hlayout);
      widg.show();
      app.exec();
      widg.hide();

     index = skyCombo.getIndex();
     printf("Index was %d\n", index);
   }

     inline int getSkyType(){return skyType;}
     inline int getRenderRes(){return renderRes;}
     inline int getNormalsOn(){return normalsOn;}
     void updateState()
     {
       printf("updating state\n");
     }
  private:
     int index;
     int skyType, renderRes, normalsOn;
     State & state;
};

#endif
