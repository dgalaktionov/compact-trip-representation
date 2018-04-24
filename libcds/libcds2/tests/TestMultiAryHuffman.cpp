/* TestMultiAryHuffman.cpp
 *
 * Copyright (C) 2015, Alberto Ordóñez, all rights reserved.
 *
 * Unity tests for MultiAryHuffman class.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <gtest/gtest.h>
#include <coders/MultiAryHuffman.h>

using namespace cds_static;


//TEST(MultiAryHuffmanTest, Test_arity_fail){
//    vector<pair<ulong,ulong>> freqs;
//    uint arity = 1;
//    ASSERT_THROW(MultiAryHuffman<ulong,uint>(vector<,3), std::runtime_error);
//
//    arity = 3;
//    ASSERT_ANY_THROW({MultiAryHuffman<ulong,uint> a(freqs,arity);});
//}

TEST(MultiAryHuffmanTest, Test_binary){

    vector<uint> pepe;
    vector<pair<ulong,ulong>> freqs;
    freqs.push_back(make_pair(0UL,1UL));
    freqs.push_back(make_pair(1UL,1UL));
    freqs.push_back(make_pair(2UL,1UL));
    freqs.push_back(make_pair(3UL,2UL));
    freqs.push_back(make_pair(4UL,2UL));
    freqs.push_back(make_pair(5UL,4UL));
    freqs.push_back(make_pair(6UL,5UL));
    freqs.push_back(make_pair(7UL,9UL));
    vector<uint> codeLengths = {4,4,4,4,3,3,2,2};
    uint arity=2;
    MultiAryHuffman<ulong,uint> mah(freqs,arity);
    ulong i=0UL;
    vector<pair<uint,uint>> codeAndLength(codeLengths.size());
    for (const auto &it: freqs){
        mah.GetCodeAndLength(it.first,codeAndLength[i].first,codeAndLength[i].second);
        EXPECT_LE(nbits<uint>(codeAndLength[i].first),codeAndLength[i].second);
        EXPECT_EQ(codeAndLength[i].second,codeLengths[i]);
        i++;
    }
    sort(codeAndLength.begin(),codeAndLength.end(),
         [](const pair<uint,uint> &a, const pair<uint,uint> &b){
            if (b.second==a.second) return a.first<b.first;
            return a.second<b.second;}
            );
    for (i=1UL;i<codeAndLength.size();i++){
        if (codeAndLength[i].second==codeAndLength[i-1].second){
            EXPECT_EQ((codeAndLength[i-1].first+1),codeAndLength[i].first);
        }
    }
}


TEST(MultiAryHuffmanTest, Test_arity_4){

    vector<uint> pepe;
    vector<pair<ulong,ulong>> freqs;
    freqs.push_back(make_pair(0UL,1UL));
    freqs.push_back(make_pair(1UL,1UL));
    freqs.push_back(make_pair(2UL,1UL));
    freqs.push_back(make_pair(3UL,2UL));
    freqs.push_back(make_pair(4UL,2UL));
    freqs.push_back(make_pair(5UL,4UL));
    freqs.push_back(make_pair(6UL,5UL));
    freqs.push_back(make_pair(7UL,9UL));
    uint arity = 4;
    MultiAryHuffman<ulong,uint> mah(freqs,arity);
    vector<uint> codeLengths = {3,3,3,3,2,2,2,1};

    ulong i=0UL;
    vector<pair<uint,uint>> codeAndLength(codeLengths.size());
    for (const auto &it: freqs){
        EXPECT_LE(nbits<uint>(codeAndLength[i].first),codeAndLength[i].second);
        mah.GetCodeAndLength(it.first,codeAndLength[i].first,codeAndLength[i].second);
        EXPECT_EQ(codeAndLength[i].second,codeLengths[i]);
        i++;
    }
    sort(codeAndLength.begin(),codeAndLength.end(),
         [](const pair<uint,uint> &a, const pair<uint,uint> &b){
             if (b.second==a.second) return a.first<b.first;
             return a.second<b.second;}
    );
    for (i=1UL;i<codeAndLength.size();i++){
        if (codeAndLength[i].second==codeAndLength[i-1].second){
            EXPECT_EQ((codeAndLength[i-1].first+1),codeAndLength[i].first);
        }
    }
}

TEST(MultiAryHuffmanVariableTest, DISABLED_Test_arity_variable){

    vector<uint> pepe;
    vector<pair<ulong,ulong>> freqs;
    freqs.push_back(make_pair(0UL,1UL));
    freqs.push_back(make_pair(1UL,1UL));
    freqs.push_back(make_pair(2UL,1UL));
    freqs.push_back(make_pair(3UL,2UL));
    freqs.push_back(make_pair(4UL,2UL));
    freqs.push_back(make_pair(5UL,4UL));
    freqs.push_back(make_pair(6UL,5UL));
    freqs.push_back(make_pair(7UL,9UL));
    uint arity = 2;
    MultiAryHuffman<ulong,uint> mah(freqs,arity);
    vector<uint> codeLengths = {3,3,3,3,3,2,2,2};

    ulong i=0UL;
    vector<pair<uint,uint>> codeAndLength(codeLengths.size());
    for (const auto &it: freqs){
        EXPECT_LE(nbits<uint>(codeAndLength[i].first),codeAndLength[i].second);
        mah.GetCodeAndLength(it.first,codeAndLength[i].first,codeAndLength[i].second);
        EXPECT_EQ(codeAndLength[i].second,codeLengths[i]);
        i++;
    }
    sort(codeAndLength.begin(),codeAndLength.end(),
         [](const pair<uint,uint> &a, const pair<uint,uint> &b){
             if (b.second==a.second) return a.first<b.first;
             return a.second<b.second;}
    );
    for (i=1UL;i<codeAndLength.size();i++){
        if (codeAndLength[i].second==codeAndLength[i-1].second){
            EXPECT_EQ((codeAndLength[i-1].first+1),codeAndLength[i].first);
        }
    }

}