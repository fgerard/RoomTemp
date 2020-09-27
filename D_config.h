#define AP_NAME "QuantumLabs"
#define AP_PASS "QuantumLabs" // OJO DEBE SER DE AL MENOS 8 CARACTERES PARA TOMAR EFECTO!!

#define I2C_speed 32000
#define SDA       21
#define SCL       22

#define QUEUE_MAX_WAIT 10000

#define TEMP_WIDNOW_SIZE 4x24
#define TEMP_DELTA 15*60

#define GES_REACTION_TIME    200       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME      200       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s). 
#define GES_QUIT_TIME     100
