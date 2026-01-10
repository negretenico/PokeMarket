#include "pch.h"
#include "../EngineLibrary/Order.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OrderTest
{
	TEST_CLASS(OrderTest)
	{
	public:
		TEST_METHOD(GivenAnOrderIsNotFullThenIGetTrue)
		{
			OrderingSystem::Order order = OrderingSystem::Order::Builder()
				.withInitialQuantity(0)
				.build();
			Assert::IsTrue(order.isFilled());
		}
		TEST_METHOD(GivenAnOrderIsNotFullThenIGetFalse)
		{
			OrderingSystem::Order order = OrderingSystem::Order::Builder()
				.withInitialQuantity(100)
				.build();
			Assert::IsFalse(order.isFilled());
		}
		TEST_METHOD(GivenAnOrderIsFilledWithToGreatAQuantityThenIGetAnFillError)
		{
			OrderingSystem::Order order = OrderingSystem::Order::Builder()
				.withInitialQuantity(1000)
				.withPrice(100)
				.build();
			Assert::IsTrue(!order.fill(1001).has_value());
		}
		TEST_METHOD(GivenAnOrderIsFilledWithAQuantityTHatIsNotToBigThenIGetASuccess)
		{
			OrderingSystem::Order order = OrderingSystem::Order::Builder()
				.withInitialQuantity(1000)
				.withPrice(100)
				.build();
			Assert::IsTrue(order.fill(500).has_value());
		}
	};
}
