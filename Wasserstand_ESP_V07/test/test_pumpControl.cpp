#include <Arduino.h>
#include <pumpControl.h>
#include <doctest.h>

TEST_CASE("selectPump tauscht Pumpen bei großem Zeitunterschied")
{
    PumpStatus ps;
    ps.pump1_operationTime = 200;
    ps.pump2_operationTime = 50;
    ps.linkPump = 0;

    PumpControl pc;
    pc.pumpA_op = 1; // Simuliere, dass Pump A läuft
    pc.pumpB_op = 0; // Simuliere, dass Pump B nicht läuft

    selectPump(ps, pc);

    CHECK(ps.linkPump == 1); // Erwartung: Pumpen werden getauscht
}

TEST_CASE("selectPump bleibt bei kleinem Zeitunterschied")
{
    PumpStatus ps;
    ps.pump1_operationTime = 100;
    ps.pump2_operationTime = 90;
    ps.linkPump = 0;

    PumpControl pc;
    pc.pumpA_op = 1; // Simuliere, dass Pump A läuft
    pc.pumpB_op = 0; // Simuliere, dass Pump B nicht läuft

    selectPump(ps, pc);
    
    
    CHECK(ps.linkPump == 0); // Erwartung: Pumpen bleiben wie sie sind
}