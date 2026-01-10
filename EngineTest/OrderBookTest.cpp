#include "pch.h"
#include "../EngineLibrary/Order.h"
#include "../EngineLibrary/Orderbook.h"
#include "../EngineLibrary/Trade.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace OrderBookTest
{
	TEST_CLASS(SingleThreaded)
	{
	public:
		TEST_METHOD(Given3MatchesThenIShouldGet3Trades)
		{
			OrderingSystem::Orderbook orderbook;
			Model::Trades trades = orderbook.addOrder(OrderingSystem::Order::Builder()
				.withInitialQuantity(100)
				.withPrice(100)
				.build());
			Assert::AreEqual(static_cast<std::size_t>(3), trades.size());
		}
		TEST_METHOD(GivenNoMatchesThenIShouldGetNoTrades)
		{
			OrderingSystem::Orderbook orderbook;
			Model::Trades trades = orderbook.addOrder(OrderingSystem::Order::Builder()
				.withInitialQuantity(100)
				.withPrice(100)
				.build());
			Assert::AreEqual(static_cast<std::size_t>(0), trades.size());
		}
		TEST_METHOD(GivenTheOrderIdExistsThenIShouldBeAbleToRemoveIt)
		{
			OrderingSystem::Orderbook orderbook;
			auto cancled = orderbook.cancelOrder(1);
			Assert::IsTrue(cancled.has_value());
		}
		TEST_METHOD(GivenTheOrderIdDoesNotExistThenISHouldGetbackAnErrorCodeIndicatingThat)
		{
			OrderingSystem::Orderbook orderbook;
			auto cancled = orderbook.cancelOrder(1);
			Assert::IsTrue(!cancled.has_value());
		}
	};
}
