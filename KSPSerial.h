/**
 * Type defs and enums for KSP serial IO
 **/

//Input enums
#define SAS 7
#define RCS 6
#define LIGHTS 5
#define GEAR 4
#define BRAKES 3
#define PRECISION 2
#define ABORT 1
#define STAGE 0

//Action group statuses
#define AGSAS      0
#define AGRCS      1
#define AGLight    2
#define AGGear     3
#define AGBrakes   4
#define AGAbort    5
#define AGCustom01 6
#define AGCustom02 7
#define AGCustom03 8
#define AGCustom04 9
#define AGCustom05 10
#define AGCustom06 11
#define AGCustom07 12
#define AGCustom08 13
#define AGCustom09 14
#define AGCustom10 15

//SAS Modes
#define SMOFF           0
#define SMSAS           1
#define SMPrograde      2
#define SMRetroGrade    3
#define SMNormal        4
#define SMAntinormal    5
#define SMRadialIn      6
#define SMRadialOut     7
#define SMTarget        8
#define SMAntiTarget    9
#define SMManeuverNode  10

//Navball Target Modes
#define NAVBallIGNORE   0
#define NAVBallORBIT    1
#define NAVBallSURFACE  2
#define NAVBallTARGET   3

// If no message received from KSP for more than X ms, go idle
#define IDLETIMER 5000

typedef struct __attribute__((__packed__)) {
	uint8_t id;             // 1
	float AP;               // 2
	float PE;               // 3
	float SemiMajorAxis;    // 4
	float SemiMinorAxis;    // 5
	float VVI;              // 6
	float e;                // 7
	float inc;              // 8
	float G;                // 9
	int32_t TAp;            // 10
	int32_t TPe;            // 11
	float TrueAnomaly;      // 12
	float Density;          // 13
	int32_t period;         // 14
	float RAlt;             // 15
	float Alt;              // 16
	float Vsurf;            // 17
	float Lat;              // 18
	float Lon;              // 19
	float LiquidFuelTot;    // 20
	float LiquidFuel;       // 21
	float OxidizerTot;      // 22
	float Oxidizer;         // 23
	float EChargeTot;       // 24
	float ECharge;          // 25
	float MonoPropTot;      // 26
	float MonoProp;         // 27
	float IntakeAirTot;     // 28
	float IntakeAir;        // 29
	float SolidFuelTot;     // 30
	float SolidFuel;        // 31
	float XenonGasTot;      // 32
	float XenonGas;         // 33
	float LiquidFuelTotS;   // 34
	float LiquidFuelS;      // 35
	float OxidizerTotS;     // 36
	float OxidizerS;        // 37
	uint32_t MissionTime;   // 38
	float deltaTime;        // 39
	float VOrbit;           // 40
	uint32_t MNTime;        // 41
	float MNDeltaV;         // 42
	float Pitch;            // 43
	float Roll;             // 44
	float Heading;          // 45
	uint16_t ActionGroups;  // 46  status bit order:SAS, RCS, Light, Gear, Brakes, Abort, Custom01 - 10
	uint8_t SOINumber;      // 47  SOI Number (decimal format: sun-planet-moon e.g. 130 = kerbin, 131 = mun)
	uint8_t MaxOverHeat;    // 48  Max part overheat (% percent)
	float MachNumber;       // 49
	float IAS;              // 50  Indicated Air Speed
	uint8_t CurrentStage;   // 51  Current stage number
	uint8_t TotalStage;     // 52  TotalNumber of stages
	float TargetDist;       // 53  Distance to targeted vessel (m)
	float TargetV;          // 54  Target vessel relative velocity
	uint8_t NavballSASMode; // 55  Combined byte for navball target mode and SAS mode
                          //   First four bits indicate AutoPilot mode:
                          //   0 SAS is off  //1 = Regular Stability Assist //2 = Prograde
                          //   3 = RetroGrade //4 = Normal //5 = Antinormal //6 = Radial In
                          //   7 = Radial Out //8 = Target //9 = Anti-Target //10 = Maneuver node
                          //   Last 4 bits set navball mode. (0=ignore,1=ORBIT,2=SURFACE,3=TARGET)
	int16_t ProgradePitch;  // 56 Pitch   Of the Prograde Vector;  int_16 ***Changed: now fix point, actual angle = angle/50*** used to be (-0x8000(-360 degrees) to 0x7FFF(359.99ish degrees)); 
	int16_t ProgradeHeading;// 57 Heading Of the Prograde Vector;  see above for range   (Prograde vector depends on navball mode, eg Surface/Orbit/Target)
	int16_t ManeuverPitch;  // 58 Pitch   Of the Maneuver Vector;  see above for range;  (0 if no Maneuver node)
	int16_t ManeuverHeading;// 59 Heading Of the Maneuver Vector;  see above for range;  (0 if no Maneuver node)
	int16_t TargetPitch;    // 60 Pitch   Of the Target   Vector;  see above for range;  (0 if no Target)
	int16_t TargetHeading;  // 61 Heading Of the Target   Vector;  see above for range;  (0 if no Target)
	int16_t NormalHeading;  // 62 Heading Of the Prograde Vector;  see above for range;  (Pitch of the Heading Vector is always 0)
} VesselData_t;

// __attribute__((__packed__))
typedef struct __attribute__((__packed__)) {
	uint8_t id;
	uint8_t M1;
	uint8_t M2;
	uint8_t M3;
} HandShakePacket_t;

typedef struct __attribute__((__packed__)) {
	uint8_t id;
	uint8_t MainControls;           // SAS RCS Lights Gear Brakes Precision Abort Stage
	uint8_t Mode;                   // 0 = stage, 1 = docking, 2 = map
	uint16_t ControlGroup;          // control groups 1-10 in 2 bytes
	uint8_t NavballSASMode;         // AutoPilot mode
	uint8_t AdditionalControlByte1;
	int16_t Pitch;                  // -1000 -> 1000
	int16_t Roll;                   // -1000 -> 1000
	int16_t Yaw;                    // -1000 -> 1000
	int16_t TX;                     // -1000 -> 1000
	int16_t TY;                     // -1000 -> 1000
	int16_t TZ;                     // -1000 -> 1000
	int16_t WheelSteer;             // -1000 -> 1000
	int16_t Throttle;               //     0 -> 1000
	int16_t WheelThrottle;          //     0 -> 1000
} ControlPacket_t;

typedef struct {
	boolean Connected;
	boolean DataSent;
	boolean DataReceived;
} KSPSerialIOState_t;


// These are the core data strcutures used to communicate with KSP
HandShakePacket_t HPacket;
VesselData_t VData;
ControlPacket_t CPacket;