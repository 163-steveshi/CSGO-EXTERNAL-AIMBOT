#include <iostream>
#include <string>

#include "memory.h"
#include "vector.h"
#include <thread>
namespace offset{
 

	// client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDB25DC;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DCDE7C;

	// engine
	constexpr ::std::ptrdiff_t dwClientState = 0x58CFC4;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;
	constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;

	// entity
	constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
	constexpr ::std::ptrdiff_t m_bDormant = 0xED;
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_lifeState = 0x25F;
	constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
	constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;

}
//return the angle that need to place the cross on enemy head from your current view angle
constexpr Vector3 CalculateAngle(
    const Vector3& localPosition,
    const Vector3& enemyPosition,
    const Vector3& viewAngles) noexcept
  {

  return ((enemyPosition - localPosition).ToAngle()-viewAngles);
  }

int main(){

  //initialize memory class
  const auto memory = Memory{"csgo.exe"};

  //get the module address
  const auto client = memory.GetModuleAddress("client.dll");
  const auto engine = memory.GetModuleAddress("engine.dll");
  while(true){

    this_thread::sleep_for(chrono:: milliseconds(1));

    //bind the key for the aim bot
    if(!GetAsyncKeyState(VK_RBUTTON))
      continue;

    //get local player: determine the player we control position as the angle reference object
    // and get local player team: determine whether the bot should aim or not: determine who is the enermy or teammate
    const auto& localPlayer = memory.Read<uintptr_t>(client + offset::dwLocalPlayer);
    const auto& localTeam = memory.Read<int32_t>(localPlayer + offset::m_iTeamNum);

    //use the third dimensional vector to determine the player's eye position = origin + viewOffset
    const auto localEyePosition = memory.Read<Vector3>(localPlayer + offset::m_vecOrigin) + memory.Read<Vector3>(localPlayer + offset::m_vecViewOffset);
    //read the client state
    const auto& clientState = memory.Read<uintptr_t>(engine + offset:: dwClientState);
    //get the view angle:
    const auto& viewAngles = memory.Read<uintptr_t>(clientState + offset:: dwClientState_ViewAngles);
    //get the anagle for weapon_recoil
    const auto &aimPunch = memory.Read<Vector3>(localPlayer + offset::m_aimPunchAngle) * 2;

    //aimbot fov
    auto bestFov = 5.f;
    auto bestAngle = Vector3{};
    for (auto i = 1; i <= 32; i++){

      //get the player entity
      const auto &player = memory.Read<uintptr_t>(client + offset::dwEntityList + i * 0x10);
      //does the entity check (decide to aim it or not)
      //decide the object is unmoved, if it is skip it
      if(memory.Read<bool>(player + offset::m_bDormant))
        continue;
      //check the health value for the enemy, if their health value is 0
      //no need to aim on it
      if(!memory.Read<std::int32_t>(player + offset::m_iHealth))
        continue;
      //check if the enemry is blocked by the object like wall
      //if it is skiping the aim
      if(!memory.Read<bool>(player + offset::m_bSpottedByMask))
        continue;

      //get the matrix that represents the player's body
      //and use it to locate the head position in 3d space
      const auto boneMatrix = memory.Read<uintptr_t>(player + offset::m_dwBoneMatrix);
      const auto playerHeadPosition = Vector3{
        memory.Read<float>(boneMatrix + 0x30 *8 + 0x0c),
        memory.Read<float>(boneMatrix + 0x30 *8 + 0x1c),
        memory.Read<float>(boneMatrix + 0x30 *8 + 0x2c),
      };
      //get the anagle with the calculate angle function
      const auto angle = CalculateAngle(localEyePosition, playerHeadPosition, viewAngles + aimPunch);

      const auto fov = hypot(angle.x, angle.y);// used to calculate the length of the hypotenuse of a right triangle given the lengths of its two other sides
      //if the measured fov is less than the default value best fov
      //update with new measured angle and fov

      if(fov < bestFov){

        bestFov = fov;
        bestAngle = angle;
      }

      //now if we have a best angle, enable the aimbot
      if(!bestAngle.IsZero())
        memory.Write<Vector3>(clientState + offset::dwClientState_ViewAngles, viewAngles + bestAngle / 3.f);
    }
  }
  return 0;
}