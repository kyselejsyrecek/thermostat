#pragma once

#include "io/thermometer.h"
#include "io/battery.h"

/*
 * IO – agregát všech vstupně/výstupních zařízení aplikace.
 *
 * Instanciuje se v app_start() a předává se do ui_init(), čímž se zajišťuje,
 * že GUI nikdy neinicializuje senzory samo – to je výhradně zodpovědnost
 * aplikační vrstvy.
 */
typedef struct {
    Thermometer *thermometer;
    Battery     *battery;
} IO;
