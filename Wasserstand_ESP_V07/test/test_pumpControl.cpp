#include <Arduino.h>
#include <pumpControl.h>
#include <doctest.h>

TEST_CASE("selectPump tauscht Pumpen bei großem Zeitunterschied")
{
    PumpControl pc;
    pc.pump1_operationTime = 200;
    pc.pump2_operationTime = 50;
    pc.linkPump = 0;

    selectPump(pc);

    CHECK(pc.linkPump == 1); // Erwartung: Pumpen werden getauscht
}

TEST_CASE("selectPump bleibt bei kleinem Zeitunterschied")
{
    PumpControl pc;
    pc.pump1_operationTime = 100;
    pc.pump2_operationTime = 90;
    pc.linkPump = 0;

    selectPump(pc);

    CHECK(pc.linkPump == 0); // Erwartung: Pumpen bleiben wie sie sind
}