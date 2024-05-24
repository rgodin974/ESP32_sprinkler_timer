
#ifndef _HMI_H_
#define _HMI_H_

#define FUNCTION_BUTTON   35
#define MANUAL_BUTTON     34

enum hmiState {IDLE, DISPLAY_MANUAL};

class Way;

class Hmi
{
  public:
    Hmi();
    void begin();
    void displayManual(void);
    void displayNextWatering(void);
    void run(void);
    bool isBusy(void);

  private:
    int m_state;
    Way *m_manualWatering;
    unsigned long m_time;
};

extern Hmi hmi;

#endif
