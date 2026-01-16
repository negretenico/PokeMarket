#include "pch.h"
#include "../EngineLibrary/Order.h"
#include "../EngineLibrary/Orderbook.h"
#include "../EngineLibrary/Side.h"
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

			// seed the book with three resting sell orders at price 100
			orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Sell)
				.withInitialQuantity(10)
				.withPrice(100)
				.withOrderId(1)
				.build());

			orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Sell)
				.withInitialQuantity(10)
				.withPrice(100)
				.withOrderId(2)
				.build());

			orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Sell)
				.withInitialQuantity(10)
				.withPrice(100)
				.withOrderId(3)
				.build());

			// incoming buy that will match the three resting orders
			Model::Trades trades = orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Buy)
				.withInitialQuantity(30)
				.withPrice(100)
				.build());

			// we should have 6 trades (3 buys, 3 sells)
			Assert::AreEqual(static_cast<std::size_t>(6), trades.size());
		}
		TEST_METHOD(GivenNoMatchesThenIShouldGetNoTrades)
		{
			OrderingSystem::Orderbook orderbook;

			// seed ask at a higher price than incoming buy
			orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Sell)
				.withInitialQuantity(50)
				.withPrice(200)
				.withOrderId(10)
				.build());

			// incoming buy at a lower price -> no matches
			Model::Trades trades = orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Buy)
				.withInitialQuantity(10)
				.withPrice(100)
				.build());

			Assert::AreEqual(static_cast<std::size_t>(0), trades.size());
		}
		TEST_METHOD(GivenTheOrderIdExistsThenIShouldBeAbleToRemoveIt)
		{
			OrderingSystem::Orderbook orderbook;

			// add an order with id 1
			orderbook.addOrder(OrderingSystem::Order::Builder()
				.withSide(Model::Side::Buy)
				.withInitialQuantity(10)
				.withPrice(100)
				.withOrderId(1)
				.build());

			auto canceled = orderbook.cancelOrder(1);
			Assert::IsTrue(canceled.has_value());
		}
		TEST_METHOD(GivenTheOrderIdDoesNotExistThenISHouldGetbackAnErrorCodeIndicatingThat)
		{
			OrderingSystem::Orderbook orderbook;
			auto canceled = orderbook.cancelOrder(999);
			Assert::IsTrue(!canceled.has_value());
		}
	};
}
