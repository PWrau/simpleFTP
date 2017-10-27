#include "stdafx.h"
#include "CppUnitTest.h"

#include "../FTPServer/Serve.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(IndexOf)
		{
			// TODO: 在此输入测试代码
            // 测试: indexOf()函数
            char* src1 = "nonfind";
            char* p1 = "NON";
            char* src2 = "findin0";
            char* p2 = "find";
            char* src3 = "findincenter";
            char* p3 = "cen";
            char* src4 = "findinlast";
            char* p4 = "last";

            int ans1 = indexOf(src1, p1);
            int ans2 = indexOf(src2, p2);
            int ans3 = indexOf(src3, p3);
            int ans4 = indexOf(src4, p4);

            Assert::AreEqual(-1, ans1);
            Assert::AreEqual(0, ans2);
            Assert::AreEqual(6, ans3);
            Assert::AreEqual(6, ans4);
            // It looks like we should write expected result in the first param position.

            

        };

        TEST_METHOD(VerbStrAndVal)
        {
            int ans1 = strcmp("USER", verbStr[0]);
            int ans2 = strcmp("PASS", verbStr[1]);
            Assert::AreEqual(0, ans1);
            Assert::AreEqual(0, ans2);
        }
        
        TEST_METHOD(VerbAndVerbListCreate)
        {
            Verb verb = createVerb(0);
            //int a = strcmp(verb.str, verbStr[0]);
            //Assert::AreEqual(0, a);
            //Assert::AreEqual('U', verb.val);
            int ans[2];
            VerbList verbList = createVerbList();
            Assert::AreEqual(VERB_NUM, (int)(verbList.length));
            for (size_t i = 0; i < verbList.length; i++)
            {
                Assert::AreNotEqual('\0', verbList.list[i].val);
                ans[i] = strcmp(verbStr[i], verbList.list[i].str);
                Assert::AreEqual(0, ans[i]);
                Assert::AreEqual(verbVal[i], verbList.list[i].val);
            }
        }
	};
}