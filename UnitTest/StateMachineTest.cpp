#include "pch.h"
#include "catch.hpp"
#include "../Utility/StateMachine/IState.h"
#include "../Utility/StateMachine/StateMachine.h"


class FakeState final : public Utility::IState
{
public:
	FakeState()
	{
	}

	~FakeState() override
	{
	}

	void Enter() override
	{
		std::cout << "Enter FakeState" << std::endl;
	}

	void Leave() override
	{
		std::cout << "Leave FakeState" << std::endl;
	}

	void Update() override
	{
		std::cout << "UpdateToFile FakeState" << std::endl;
		OnDone();
	}

	std::function<void()> OnDone;
};

TEST_CASE("state machine test", "[Utility]")
{
	 auto machine = new Utility::StateMachine;
 
	 const auto fake = new FakeState;

	 bool bDone = false;
	 fake->OnDone = [&]()
	 {
	 	std::cout << "state done" << std::endl;
		bDone = true;
	 };
 
	 const std::shared_ptr<Utility::IState> state(fake);
	 machine->Push(state);
	 machine->Update();
 
	 delete machine;
	 
	REQUIRE(bDone== true);
}
