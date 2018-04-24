/* MultiAryHuffman.h
 *
 * Copyright (C) 2015, Alberto Ordóñez, all rights reserved.
 *
 * Builds a canonical Huffman encoding for a set of frequencies.
 * The arity of the Huffman tree can be specified at construction time.
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

#ifndef MULTIARYHUFFMAN_H_
#define MULTIARYHUFFMAN_H_

#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
using namespace std;

namespace cds_static{


    template<typename T=uint> ulong nbits(T value){
       ulong res = 0UL;
        while (value) {
            res++;
            value >>= 1;
        }
        return res;
    }

    template <typename id_type> class HuffmanNode{
    public:
        HuffmanNode(id_type _id, ulong _freq):id(_id),freq(_freq){}
        HuffmanNode(id_type _id):id(_id),freq(0UL){}
        ~HuffmanNode(){};
        id_type GetId(){return id;}
        ulong GetFreq(){return freq;}

        void AddChild(HuffmanNode<id_type> &&child){
            freq+=child.GetFreq();
            children.push_back(std::move(child));
        }
        typename vector<HuffmanNode<id_type>>::iterator GetChildrenIterator(){
            return children.begin();
        }
        typename vector<HuffmanNode<id_type>>::iterator GetChildrenEndIterator(){
            return children.end();
        }

    protected:
        id_type id;
        ulong freq;
        vector<HuffmanNode<id_type>> children;
    };

    template <typename id_type=ulong, typename code_type=uint> class MultiAryHuffman{
    protected:
        struct SymbolCodeLength{
            SymbolCodeLength(id_type _symbol, code_type ct, uint len):symbol(_symbol),code(ct),length(len){}
            SymbolCodeLength(id_type _symbol, uint len):symbol(_symbol),code(static_cast<code_type>(0)),length(len){}
            SymbolCodeLength():symbol(static_cast<id_type>(-1)),code(static_cast<code_type>(0)),length(0U){}
            ~SymbolCodeLength(){};
            id_type symbol;
            code_type code;
            uint length;
        };
    public:

        //Receives a vector containing pairs of <symbol,frequency> and the arity of the huffman tree.
        //Builds a canonnical huffman encoding with the given arity for the frequencies given.
        MultiAryHuffman(vector<pair<id_type,ulong>> &freqs, uint _arity):arity(_arity){
            //arity must be a power of 2

            if (arity<2 || ((1UL<<(nbits(arity)-1)) != arity)) {
                cerr << "Arity must be a power of 2 >=2! (The parameter given is " << arity << ")" << endl;
                throw runtime_error("Arity must be a power of 2 >=2! (The parameter given is " +  [](uint arity){stringstream ss;ss << arity;return ss.str();}(arity)+ ")");
            }else if (freqs.size()<3){
                    cerr << "The input vector of freqs has size " << freqs.size() << "!. At least 3 is required!" << endl;
                throw runtime_error("The input vector of freqs has size " + [](ulong arity){stringstream ss;ss << arity;return ss.str();}(freqs.size()) + "!. At least 3 is required!");
            }else {
                if (![](const vector<pair<id_type,ulong>> &freqs) {
                    for (ulong i = 1; i < freqs.size(); i++) {
                        if (freqs[i] < freqs[i - 1]) return false;
                    }
                    return true;
                }(freqs)){
                    sort(freqs.begin(),freqs.end(),[](const pair<ulong,ulong> &a, const pair<ulong,ulong> &b){return a.second<b.second;});
                }
                BuildHuffman(freqs);
            }
        }
        //Given a symbol, returns its associated code and codelength.
        //codelength is the number of chunks of "log_2(arity)" bits of the code.
        void GetCodeAndLength(const id_type &symbol, code_type &code, uint &length){
            const auto hash_content = symbol_to_code.find(symbol);
            if (hash_content==symbol_to_code.end()){
                code = static_cast<code_type>(-1);
                length = 0UL;
            }else{
                code = hash_content->second.code;
                length = hash_content->second.length;
            }
        }

    protected:
        MultiAryHuffman(uint _arity):arity(_arity){}

        //stores, for each symbol, its code and codelength.
        unordered_map<id_type,SymbolCodeLength> symbol_to_code;
        //arity of the huffman tree
        uint arity;

        //Runs the huffman algorithm and ends up initializing the hash table symbols_to_code
        void BuildHuffman(const vector<pair<id_type,ulong>> &freqs){
            //define queues
            queue<HuffmanNode<id_type>> queue1;
            queue<HuffmanNode<id_type>> queue2;
            ulong totalFreq=0UL;
            //init queues

            InitQueue(freqs,queue1,totalFreq);
            //run construction algorithm
            RunHuffmanAlgorithm(queue1,queue2);
            //compute code_lengths
            assert(queue2.front().GetFreq()==totalFreq);
            ComputeCodelengths(queue2.front(),0U);
            ComputeCodes();
        }

        //Initializes the queue with the symbols and frequencies given in the vector freqs. Note this symbols are
        //sorted in increasing order by frequency.
        void InitQueue(const vector<pair<id_type,ulong>> &freqs, queue<HuffmanNode<id_type>> & queue1, ulong &totalFreq){
            totalFreq = 0UL;
            for (const auto &it: freqs){
                queue1.push(HuffmanNode<id_type>(it.first,it.second));
                totalFreq+=it.second;
            }
            assert(queue1.size()==freqs.size());
        }

        //Builds the huffman tree given two queues. Initially queue1 contains all the symbols with their frequencies
        //sorted in increasing order. queue2 is initially empty.
        virtual void RunHuffmanAlgorithm(queue<HuffmanNode<id_type>> & queue1, queue<HuffmanNode<id_type>> &queue2){

            while(queue1.size()!=0 || queue2.size()!=1){
                uint node_count = 0;
                uint supLimit = ((queue1.size()+queue2.size())>=arity)?arity:static_cast<uint>(queue1.size()+queue2.size());
                HuffmanNode<id_type> internal(-1UL);
                while(node_count++<supLimit){
                    internal.AddChild(PickMinFromQueues(queue1,queue2));
                }
                queue2.push(internal);
            }
            assert(queue1.empty());
            assert(queue2.size()==1);
        }
        //Given two queues, pops the minimum of both (and returns it).
        //Requires: at least one queue must be non empty
        HuffmanNode<id_type> PickMinFromQueues(queue<HuffmanNode<id_type>> &queue1, queue<HuffmanNode<id_type>> &queue2){
            if (queue1.empty() && queue2.empty()){
                cerr << "Error::cannot be both empty" << endl;
            }
            queue<HuffmanNode<id_type>> *selected;
            if (queue1.empty()) {
                selected = &queue2;
            }else if (queue2.empty()) {
                selected = &queue1;
            }else if (queue1.front().GetFreq()<=queue2.front().GetFreq()){
                selected = &queue1;
            }else{
                selected = &queue2;
            }
            HuffmanNode<id_type> min_node = selected->front();
            selected->pop();
            return min_node;
        }
        //Given the huffman tree, computes the length of the code associated with each symbol.
        void ComputeCodelengths(HuffmanNode<id_type> &root, uint codeLen){
            if (root.GetChildrenEndIterator()==root.GetChildrenIterator()){
                symbol_to_code[root.GetId()]=SymbolCodeLength(root.GetId(),codeLen);
            }else{
                for (auto iter = root.GetChildrenIterator(); iter!=root.GetChildrenEndIterator();++iter){
                    ComputeCodelengths(*iter,codeLen+1);
                }
            }
        }

        //computes a canonical encoding
        void ComputeCodes(){
            vector<SymbolCodeLength> encoding;
            for (auto &iter: symbol_to_code){
                encoding.push_back(iter.second);
            }
            //sort the codes by length
            sort(encoding.begin(),encoding.end(),[](const SymbolCodeLength &a,const SymbolCodeLength &b){return a.length<b.length;});

            code_type code = static_cast<code_type>(0);
            encoding[0].code = code++;
            uint shift_arity = nbits(arity)-1;
            for (ulong i=1;i<encoding.size();i++){
                if (encoding[i].length==encoding[i-1].length){
                    encoding[i].code = code++;
                }else{
                    int diff = encoding[i].length-encoding[i-1].length;
                    while(diff--){
                        code = code << shift_arity;
                    }
                    encoding[i].code = code++;
                }
            }
            for (const auto &iter: encoding){
//                cout << iter.symbol << "," << iter.code << "," << iter.length << endl;
                symbol_to_code[iter.symbol] = iter;
            }
        }

    };

    template <typename id_type=ulong, typename code_type=uint> class MultiAryHuffmanVariable:
            public MultiAryHuffman<id_type,code_type>{

        //friend class MultiAryHuffman<id_type,code_type>;

    public:
        //Receives a vector containing pairs of <symbol,frequency> and the arity of the huffman tree.
        //Builds a canonnical huffman encoding with the given arity for the frequencies given.
        MultiAryHuffmanVariable(vector<pair<id_type,ulong>> &freqs, uint _arity);

        virtual ~MultiAryHuffmanVariable(){}
        //Builds the huffman tree given two queues. Initially queue1 contains all the symbols with their frequencies
        //sorted in increasing order. queue2 is initially empty.
        virtual void RunHuffmanAlgorithm(queue<HuffmanNode<id_type>> & queue1, queue<HuffmanNode<id_type>> &queue2){

            while(queue1.size()!=0 || queue2.size()!=1){
                uint node_count = 0;
                uint supLimit = ((queue1.size()+queue2.size())>=this->arity)?this->arity:static_cast<uint>(queue1.size()+queue2.size());
                HuffmanNode<id_type> internal(-1UL);
                while(node_count++<supLimit){
                    internal.AddChild(PickMinFromQueues(queue1,queue2));
                }
                auto last_freq = (--internal.GetChildrenEndIterator())->GetFreq();
                while(true){
                    auto ret = GetQueueWithMin(queue1,queue2);
                    if (ret.front().GetFreq()==last_freq) {
                        internal.AddChild(ret.front());
                        ret.pop();
                    }else{
                        break;
                    }
                }
                queue2.push(internal);
            }
            assert(queue1.empty());
            assert(queue2.size()==1);
        }
        //Given two queues, pops the minimum of both (and returns it).
        //Requires: at least one queue must be non empty
        queue<HuffmanNode<id_type>> GetQueueWithMin(queue<HuffmanNode<id_type>> &queue1, queue<HuffmanNode<id_type>> &queue2){
            if (queue1.empty() && queue2.empty()){
                cerr << "Error::cannot be both empty" << endl;
            }
            queue<HuffmanNode<id_type>> *selected;
            if (queue1.empty()) {
                selected = &queue2;
            }else if (queue2.empty()) {
                selected = &queue1;
            }else if (queue1.front().GetFreq()<=queue2.front().GetFreq()){
                selected = &queue1;
            }else{
                selected = &queue2;
            }
            return selected;
        }

    };
    template <typename id_type, typename code_type> MultiAryHuffmanVariable<id_type,code_type>::
    MultiAryHuffmanVariable(vector<pair<id_type,ulong>> &freqs, uint _arity):
            MultiAryHuffman<id_type,code_type>(_arity){
        //arity must be a power of 2

        if (this->arity<2 || ((1UL<<(nbits(this->arity)-1)) != this->arity)) {
            cerr << "Arity must be a power of 2 >=2! (The parameter given is " << this->arity << ")" << endl;
            throw runtime_error("Arity must be a power of 2 >=2! (The parameter given is " +  [](uint arity){stringstream ss;ss << arity;return ss.str();}(this->arity)+ ")");
        }else if (freqs.size()<3){
            cerr << "The input vector of freqs has size " << freqs.size() << "!. At least 3 is required!" << endl;
            throw runtime_error("The input vector of freqs has size " + [](ulong freqs_size){stringstream ss;ss << freqs_size;return ss.str();}(freqs.size()) + "!. At least 3 is required!");
        }else {
            if (![](const vector<pair<id_type,ulong>> &freqs) {
                for (ulong i = 1; i < freqs.size(); i++) {
                    if (freqs[i] < freqs[i - 1]) return false;
                }
                return true;
            }(freqs)){
                sort(freqs.begin(),freqs.end(),[](const pair<ulong,ulong> &a, const pair<ulong,ulong> &b){return a.second<b.second;});
            }
            BuildHuffman(freqs);
        }
    }

};

#endif