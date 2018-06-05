#include "pch.h"


#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../Utility/StateMachine.h"
#include "../curl/curl.h"
#include "../Utility/IDownloadable.h"
#include "../Utility/CurlHttp.h"
#include <thread>
#include <mutex>
#include "../Utility/md5.h"
#include "../Utility/HttpDownload.h"
#include <future>
#include "../Utility/FileTool.h"


std::mutex gMutex;

void f1(int n)
{
	for (int i = 0; i < 5; ++i)
	{
		std::cout << "Thread " << n << " executing\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void f2(int& n)
{
	//std::lock_guard<std::mutex> mLock(gMutex); // new lock

	//gMutex.lock(); //old

	for (int i = 0; i < 5; ++i)
	{
		std::cout << "Thread 2 executing\n";
		++n;
		//std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(500));

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return;
	}
	//gMutex.unlock();
}

//TEST_CASE("thread test", "[thread]")
//{
int n = 0;

//std::thread t1; // t1 is not a thread
//std::thread t2(f1, n + 1); // pass by value
//std::thread t3(f2, std::ref(n)); // pass by reference
//std::thread t4(f2, std::ref(n)); // pass by reference
// std::thread t4(std::move(t3)); // t4 is now running f2(). t3 is no longer a thread
//t3.join();
//t4.join();
//std::cout << "Final value of n is " << n << '\n';

//auto t233 = std::thread(f1,n); // t4 is now running f2(). t3 is no longer a thread
//}


TEST_CASE("MD5_C_Style", "[Utility]")
{
	std::FILE* fp = std::fopen("dummy.txt", "rb");
	assert(fp);

	std::fseek(fp, 0, SEEK_END); // seek to end
	const std::size_t filesize = std::ftell(fp);

	std::fseek(fp, 0, SEEK_SET); // seek to start
	std::vector<uint8_t> buffer(filesize);

	std::fread(buffer.data(), sizeof(uint8_t), buffer.size(), fp);

	MD5 md5;
	md5.update(buffer.data(), buffer.size());
	md5.finalize();

	const auto strMd5 = md5.hexdigest();

	std::fclose(fp);
	std::printf("i've read %zi bytes\n", filesize);

	std::string a = "327c478cce88005e6ad0d5ad10811b11";

	//auto result = std::strcmp(a.c_str(), strMd5.c_str());

	auto aa = a.compare(strMd5);

	//auto r = a == strMd5;

	std::cout << strMd5 << std::endl;
}


TEST_CASE("MD5-2", "[Utility]")
{
	std::ifstream infile("testfile.txt", std::ios::in | std::ios::ate); //read mode | read to end

	if (!infile.is_open())
	{
		int i = 0;
		assert("open file error, testfile.txt");
		return;
	}

	const auto size = infile.tellg();

	std::vector<unsigned char> buffer(size);

	infile.seekg(0);
	infile.read(reinterpret_cast<char*>(buffer.data()), size);
	infile.close();

	MD5 md5;
	md5.update(buffer.data(), buffer.size());
	md5.finalize();
	const auto strmd5 = md5.hexdigest();

	auto oldmd5 = "9d5826b31abc288c9587276fc74d98a1";

	std::ofstream outfile("md5list.txt", std::ofstream::out | std::ofstream::app); //write mode | write data from eof 

	outfile << buffer.data() << std::endl;
	outfile << "dm5 = " << strmd5.data() << std::endl << std::endl;

	outfile.close();
}


// template <typename F>
// struct make_tuple_of_params;
//
// //////////////////////////////////////////////////////////
// template <typename Ret, typename... Args>
// struct make_tuple_of_params<Ret(*)(Args ...)>
// {
// 	using type = std::tuple<Args...>;
// };
//
// //////////////////////////////////////////////////////////
// template <typename F>
// auto magic_wand(F f)
// {
// 	return make_tuple_of_params<F>::type();
// }


// class ReadLocalVersionFile
// {
// public:
// 	ReadLocalVersionFile(const std::string path){}
// 	~ReadLocalVersionFile() {}
//
// 	auto GetCurrectVersion() const { return _Version; }
//
// private:
// 	std::string _Version;
// 	//read file in local path
// };


class FakeDonwload : public Utility::IDownloadable
{
public:
	FakeDonwload(int file_size): _FileSize(file_size)
	{
	}

	FakeDonwload(int file_size, int downloaded) : _FileSize(file_size), _DownloadedSize(downloaded)
	{
	}

	virtual ~FakeDonwload()
	{
	}

	std::shared_ptr<Utility::ReceiverFacade> Start(std::string url) override
	{
		return _ReceiverFacade;
	}


	void ReceiveData(char* data, int length)
	{
		_DownloadedSize += length;
		if (_DownloadedSize >= _FileSize)
		{
			_ReceiverFacade->InvokeDownloadDone(true);
		}

		_ReceiverFacade->InvokeWriteData(data, 1, length);
		_ReceiverFacade->InvokeProgress(_FileSize, _DownloadedSize);
	}

private:
	std::shared_ptr<Utility::ReceiverFacade> _ReceiverFacade{new Utility::ReceiverFacade()};
	int _FileSize{0};
	int _DownloadedSize{0};
};

SCENARIO("fake download", "[test-1]")
{
	//arrange
	std::vector<char> file(1, 0);

	FakeDonwload download(file.size()); //user

	//act
	auto facade = download.Start("");

	facade->bindWriteData
	(
		[&](char* buffer, size_t size, size_t nmemb)-> size_t
		{
			return nmemb;
		}
	);

	facade->bindProgresser
	(
		[=](int total, int downloaded)-> int
		{
			const auto percent = downloaded * 100.0 / total;
			std::cout << "fake percent=" << percent << "%" << "\r";
			return 0;
		}
	);

	bool done = false;
	facade->bindReceiverDone
	(
		[&](bool result)
		{
			done = result;
		}
	);

	download.ReceiveData(file.data(), 1);

	//assert
	REQUIRE(done == true);
}


// SCENARIO("download muti file", "[test-1]")
// {
// 	GIVEN("i have url list")
// 	{
// 		std::vector<std::tuple<std::string, std::string>> urls;
//
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file0.txt");
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file1.txt");
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file2.txt");
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file3.txt");
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file4.txt");
// 		urls.emplace_back("http://tpdb.speed2.hinet.net/test_040m.zip", "file5.txt");
//
// 		WHEN("i ready download")
// 		{
// 			Utility::HttpDownload download; //user
//
// 			THEN("i receive file in disk")
// 			{
// 				for (auto&& tuple : urls)
// 				{
// 					const auto url = std::get<0>(tuple);
// 					auto filename = std::get<1>(tuple);
//
// 					auto& receiver = download.Start(url);
//
// 					FILE* fp = std::fopen(filename.c_str(), "w");
//
// 					receiver.bindWriteData
// 					(
// 						[&](char* buffer, size_t size, size_t nmemb)-> size_t
// 						{
// 							return fwrite(buffer, size, nmemb, fp);
// 						}
// 					);
//
// 					receiver.bindProgresser
// 					(
// 						[=](int total, int downloaded)-> int
// 						{
// 							const auto percent = downloaded * 100.0 / total;
// 							//std::cout << "percent=" << percent << "\r";
// 							return 0;
// 						}
// 					);
//
//
// 					bool done = false;
// 					receiver.bindReceiverDone
// 					(
// 						[&](bool result)
// 						{
// 							fclose(fp);
// 							done = result;
// 							//std::cout << std::endl;
// 						}
// 					);
//
// 					while (!done)
// 					{
// 					}
//
// 					REQUIRE(done == true);
// 				}
// 			}
// 		}
// 	}
// }

/*
Feature: resume download
in order to 節省更新時間
as s 玩家
i want to be 可以續傳
*/
SCENARIO("fake download break", "[test-1]")
{
	GIVEN("i start download")
	{
		std::vector<char> sourceFile;
		std::vector<char> localFile;
		bool done = false;
		WHEN("i get remote file size")
		{
			sourceFile = {'1', '2', '3', '4', '5'};
			FakeDonwload download(sourceFile.size());
			auto facade = download.Start("");

			AND_WHEN("write data to local file")
			{
				facade->bindWriteData
				(
					[&](char* buffer, size_t size, size_t nmemb)-> size_t
					{
						localFile.push_back(*buffer);
						return nmemb;
					}
				);

				facade->bindProgresser
				(
					[=](int total, int downloaded)-> int
					{
						const auto percent = downloaded * 100.0 / total;
						std::cout << "fake percent=" << percent << "%" << "\r";
						return 0;
					}
				);

				facade->bindReceiverDone
				(
					[&](bool result)
					{
						done = result;
					}
				);

				AND_WHEN("i stop download")
				{
					download.ReceiveData(sourceFile.data(), 1);

					REQUIRE_FALSE(done);

					THEN("i get unfinish file size")
					{
						REQUIRE(localFile.size() == 1);
						REQUIRE(localFile.at(0) == '1');

						AND_THEN("i get leftover size")
						{
							int leftover = sourceFile.size() - localFile.size();
							REQUIRE(leftover == 4);
						}
					}
				}
			}
		}
	}
}

SCENARIO("fake resume download", "[test-1]")
{
	GIVEN("i restart download")
	{
		std::vector<char> sourceFile = {'1', '2', '3', '4', '5'};
		std::vector<char> localFile = {'1'};
		std::vector<char> leftoverFile = { '2', '3', '4', '5' };

		bool done = false;
		WHEN("i open local unifish file")
		{
			AND_WHEN("i start here keep on download to file end")
			{
				FakeDonwload download(sourceFile.size(), localFile.size());
				auto facade = download.Start("");
				facade->bindWriteData
				(
					[&](char* buffer, size_t size, size_t nmemb)-> size_t
					{
						localFile.push_back(*buffer);
						return nmemb;
					}
				);

				facade->bindProgresser
				(
					[=](int total, int downloaded)-> int
					{
						const auto percent = downloaded * 100.0 / total;
						std::cout << "fake percent=" << percent << "%" << "\r";
						return 0;
					}
				);

				facade->bindReceiverDone
				(
					[&](bool result)
					{
						done = result;
					}
				);

				THEN("i get complete file")
				{
					download.ReceiveData(leftoverFile.data(), 1);
					download.ReceiveData(leftoverFile.data(), 1);
					download.ReceiveData(leftoverFile.data(), 1);
					download.ReceiveData(leftoverFile.data(), 1);

					REQUIRE(localFile.size() == sourceFile.size());

					REQUIRE(done);
				}
			}
		}
	}
}


TEST_CASE("download one file", "[test-1]")
{
	//arrange
	std::string url = "http://tpdb.speed2.hinet.net/test_040m.zip";

	//std::string url = "https://dl.google.com/dl/android/studio/install/3.0.1.0/android-studio-ide-171.4443003-windows.exe";
	std::string filename = "testfile1.txt";
	FILE* fp = std::fopen(filename.c_str(), "w");


	//act
	Utility::HttpDownload download; //user
	auto size = FileTool::GetFileSize_C(filename);
	auto size1 = FileTool::GetFileSize_CPlusPlus(filename);

	auto facade = download.Start(url);

	facade->bindWriteData
	(
		[&](char* buffer, size_t size, size_t nmemb)-> size_t
		{
			auto r = fwrite(buffer, size, nmemb, fp);
			return r;
		}
	);

	facade->bindProgresser
	(
		[=](int total, int downloaded)-> int
		{
			const auto percent = downloaded * 100.0 / total;
			std::cout << "percent=" << percent << "\r";
			return 0;
		}
	);

	bool done = false;
	facade->bindReceiverDone
	(
		[&](bool result)
		{
			fclose(fp);
			done = result;
		}
	);

	while (!done) //main loop
	{
	}

	//assert
	REQUIRE(done == true);
}

TEST_CASE("ReceiveProgress before bind", "[curl]")
{
	auto test = Utility::ReceiveProgress();

	test.Bind([=](int total, int downloaded)-> int
	{
		return 0;
	});

	auto r = test.Invoke(1, 1);

	REQUIRE(r == 0);
}

TEST_CASE("ReceiveProgress before invoke", "[curl]")
{
	auto test = Utility::ReceiveProgress();

	auto r = test.Invoke(1, 1); //true 

	test.Bind([=](int total, int downloaded)-> int
	{
		return 0;
	});

	REQUIRE(r == 0);
}

TEST_CASE("ReceiveDone before bind", "[curl]")
{
	auto test = Utility::ReceiveDone();

	bool done = false;
	test.Bind([&](bool result)
	{
		done = true;
	});

	test.Invoke(true);

	REQUIRE(done == true);
}

TEST_CASE("ReceiveDone before invoke", "[curl]")
{
	auto test = Utility::ReceiveDone();

	test.Invoke(true); //true 

	bool done = false;
	test.Bind([&](bool result)
	{
		done = true;
	});

	REQUIRE(done == true);
}

TEST_CASE("ReceiveWriteData before bind", "[curl]")
{
	auto test = Utility::ReceiveWriteData();

	FILE* fp = std::fopen("testfile2.txt", "w");

	char buffer[]{'1', '2', '3', '\0'};

	auto write_count = test.Bind
	(
		[&](char* b, size_t size, size_t nmemb)-> size_t
		{
			return fwrite(b, size, nmemb, fp);
		}
	);

	REQUIRE(write_count == 0);

	write_count = test.Invoke(buffer, 1, sizeof(buffer));
	REQUIRE(write_count == 4);

	fclose(fp);
}

TEST_CASE("ReceiveWriteData before invoke", "[curl]")
{
	auto test = Utility::ReceiveWriteData();
	FILE* fp = std::fopen("testfile3.txt", "w");
	char buffer[]{'1', '2', '3', '4', '5', '\0'};

	auto write_count = test.Invoke(buffer, 1, sizeof(buffer)); //true 

	REQUIRE(write_count == 6);

	write_count = test.Bind
	(
		[&](char* b, size_t size, size_t nmemb)-> size_t
		{
			return fwrite(b, size, nmemb, fp);
		}
	);

	REQUIRE(write_count == 6);
}


bool is_prime(int x)
{
	for (int i = 2; i < x; ++i)
	{
		if (x % i == 0)
		{
			return false;
		}
	}

	return true;
}

void is_prime2(int x)
{
	for (int i = 2; i < x; ++i)
	{
		if (x % i == 0)
		{
			std::cout << 0 << std::endl;
		}
	}

	std::cout << 1 << std::endl;
}

void sleep(int milisec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milisec));
}

TEST_CASE("async", "[curl]")
{
	// thread()
	// {
	// 	class (callback)
	// 	{
	// 		std::future<bool> fut1 = std::async(std::launch::async, is_prime, 334214467);
	// 		std::future<bool> fut2 = std::async(std::launch::async, is_prime, 334214467);
	// 		std::future<bool> fut3 = std::async(std::launch::async, is_prime, 334214467);
	//
	// 		if (fut1.get() && fut2.get() && fut3.get()) //await
	// 		{
	// 			callback.invoke();
	// 		}
	//
	// 	}
	// }


	std::future<void> fut2 = std::async(std::launch::async, is_prime2, 334214467);

	sleep(200);
	std::cout << "Do something in main thread ...\n";
	sleep(200);
	std::cout << "Do something in main thread ...\n";
	sleep(200);
	std::cout << "Do something in main thread ...\n";

	fut2.get();

	// bool r = fut.get();         // retrieve return value
	//
	// if (r) {
	// 	std::cout << "It is prime!\n";
	// }
	// else {
	// 	std::cout << "It is not prime.\n";
	// }
}