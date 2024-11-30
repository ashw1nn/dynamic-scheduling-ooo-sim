#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <sstream>

#define DEBUG 0

#define endl "\n"
using namespace std;

ofstream log_file("pipeline_log.txt");


template <typename T> void print_vector(const vector<T> &v, ostream &os = cout, const string &seper = " ") {
    os << "----------------------------------------------" << endl;
    for (const auto &elem : v) {
        os << elem << seper;
    }
    os << "----------------------------------------------" << endl;
}

struct ROBEntry
{
    int tag;
    u_int32_t pc;
    int operation;
    int state;
    int sc1;
    bool sc1_ready = true;
    int sc2;
    bool sc2_ready = true;
    int dest;
    bool operand_ready = false;
    int execution_cycles = -1;
    bool completed = false;

    // Cycle track
    mutable int sc1_original = -1, src2_original = -1;
    mutable int if_cycle_start = -1, if_cycle_duration = -1;
    mutable int id_cycle_start = -1, id_cycle_duration = -1;
    mutable int is_cycle_start = -1, is_cycle_duration = -1;
    mutable int ex_cycle_start = -1, ex_cycle_duration = -1;
    mutable int wb_cycle_start = -1, wb_cycle_duration = -1;

    friend ostream& operator<<(ostream& os, const ROBEntry& entry) {
        int new_tag = entry.tag - 1000;
        os << "Tag: " << new_tag << ", PC: " << entry.pc << ", Op: " << entry.operation
           << ", State: " << entry.state << ", SC1: " << entry.sc1 << ", SC1 Ready: " << entry.sc1_ready
           << ", SC2: " << entry.sc2 << ", SC2 Ready: " << entry.sc2_ready << ", Dest: " << entry.dest
           << ", Operand Ready: " << entry.operand_ready << ", Exec Cycles: " << entry.execution_cycles
           << ", Completed: " << entry.completed;
        return os;
    }


    bool operator==(const ROBEntry& other) const {
    return (tag == other.tag) &&
           (pc == other.pc) &&
           (operation == other.operation) &&
           (state == other.state) &&
           (sc1 == other.sc1) &&
           (sc2 == other.sc2) &&
           (dest == other.dest) &&
           (operand_ready == other.operand_ready) &&
           (execution_cycles == other.execution_cycles) &&
           (completed == other.completed);
    }
};

void printROBEntry(ROBEntry entry) {
    cout << "Tag: " << (entry.tag-1000) << ", PC: " << entry.pc << ", Op: " << entry.operation
         << ", State: " << entry.state << ", SC1: " << entry.sc1 << ", SC1 Ready: " << entry.sc1_ready
         << ", SC2: " << entry.sc2 << ", SC2 Ready: " << entry.sc2_ready << ", Dest: " << entry.dest
         << ", Operand Ready: " << entry.operand_ready << ", Exec Cycles: " << entry.execution_cycles
         << ", Completed: " << entry.completed << endl;
}

std::string outputFormattedROBEntry(const ROBEntry& entry) {
    std::ostringstream ss;
    ss << entry.tag << " fu{" << entry.operation << "} src{" << entry.sc1_original << "," << entry.src2_original << "} dst{" << entry.dest 
       << "} IF{" << entry.if_cycle_start << ",1} ID{" << entry.id_cycle_start << ",1} IS{" << entry.is_cycle_start << "," << entry.is_cycle_duration 
       << "} EX{" << entry.ex_cycle_start << "," << entry.ex_cycle_duration << "} WB{" << entry.wb_cycle_start << ",1}\n";
    return ss.str();
}


unordered_map<string, int> STATES = {
    {"F", 0},
    {"D", 1},
    {"S", 2},
    {"E", 3},
    {"W", 4},
};


// class ROB
// {
// public:
//     ROB(size_t size) : head(0), tail(0), count(0), max_size(size) {
//         buffer.resize(max_size);
//     }

//     bool addInstruction(const ROBEntry& entry) {
//         if (isFull()) {
//             return false;
//         }
//         // cout <<  "Overwritten " << buffer[tail].tag << endl;
//         buffer[tail] = entry; // Overwrite existing entries
//         tail = (tail + 1) % max_size;
//         ++count;
//         return true;
//     }

//     bool removeInstruction() {
//         if (isEmpty()) {
//             return false;
//         }
//         head = (head + 1) % max_size;
//         --count;
//         return true;
//     }

//     void updateInstruction(const ROBEntry& entry) {
//         for(size_t i = 0; i < count; ++i) {
//             size_t index = (head + i) % max_size;
//             if(buffer[index].tag == entry.tag) {
//                 buffer[index] = entry;
//                 return;
//             }
//         }
//         // cout << "Instruction not in ROB " << entry.tag << endl;
//         // assert(false);
//     }

//     void updateRegisterReadiness(int dest, int compTag) {
//         if(dest == -1) return;
//         for(size_t i = 0; i < count; ++i) {
//             size_t index = (head + i) % max_size;
//             if(buffer[index].state == STATES["S"] && buffer[index].operand_ready == false) {
//                 if(compTag == 1107) {
//                     // cout<<"YESSS@@@@@@@"<<endl;
//                     // cout << "Tags in buffer " << buffer[index].tag << endl;
//                 }
//                 if(buffer[index].sc1 == compTag) {
//                     buffer[index].sc1_ready = true;
//                     if(DEBUG) log_file << "SC1 Ready for tag " << buffer[index].tag << endl;
//                 }
//                 if(buffer[index].sc2 == compTag) {
//                     buffer[index].sc2_ready = true;
//                     if(DEBUG) log_file << "SC2 Ready for tag " << buffer[index].tag << endl;
//                 }
//                 // if(!buffer[index].sc1 != compTag && !buffer[index].sc2 != compTag) {
//                 //     log_file << "Completed inst tag " << compTag  << " doesnt match for isssue inst tag "
//                 //              <<  buffer[index].tag << endl; 
//                 // }
//             }
//         }
//     }

//     void moveToDispatch(vector<ROBEntry> &dispatch_list, const size_t &N, const size_t &cycle) {
//         for(size_t i = 0; i < count && dispatch_list.size() < 2*N; ++i) {
//             size_t index = (head + i) % max_size;
//             if(buffer[index].state == STATES["F"]) {

//                 buffer[index].state = STATES["D"];

//                 if(buffer[index].if_cycle_duration == -1) {
//                     buffer[index].if_cycle_duration = 1;
//                     buffer[index].id_cycle_duration = 1;
//                 }

//                 dispatch_list.push_back(buffer[index]);
//                 // log_file << "Advanced to Dispatch: " << buffer[index] << endl;
//             }
//         }
//     }

//     void WakeUp(vector<pair<int, int>> &register_file, vector<ROBEntry> &issue_list) {
//         for(size_t i = 0; i < count; ++i) {
//             size_t index = (head + i) % max_size;
//             if(buffer[index].state == STATES["S"]) {
//                 if(buffer[index].sc1_ready && buffer[index].sc2_ready && !buffer[index].operand_ready) {
//                     buffer[index].operand_ready = true;
//                     auto it = find_if(issue_list.begin(), issue_list.end(), [&](const ROBEntry& entry) {
//                         return entry.tag == buffer[index].tag;
//                     });
//                     if (it != issue_list.end()) {
//                         *it = buffer[index];
//                         if(DEBUG) log_file << "Updated tag " << buffer[index].tag << " in issue_list." << endl;
//                     }
//                 } 
//             }
//         }
//     }

//     void retireInstruction(const size_t &cycle) {
//         ROBEntry entry = buffer[head];
//         if(entry.wb_cycle_duration == -1) {
//             entry.wb_cycle_duration = 1;
//         }

        // printf("%d fu{%d} src{%d,%d} dst{%d} IF{%d,%d} ID{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d}\n",
        //        entry.tag, entry.operation, entry.sc1_original, entry.src2_original, entry.dest,
        //        entry.if_cycle_start, 1,
        //        entry.id_cycle_start, 1,
        //        entry.is_cycle_start, entry.is_cycle_duration,
        //        entry.ex_cycle_start, entry.ex_cycle_duration,
        //        entry.wb_cycle_start, 1);
//     }

//     bool isFull() const {
//         return count == max_size;
//     }

//     bool isEmpty() const {
//         // return (head==tail);
//         return count == 0;
//     }

//     ROBEntry getHead() {
//         return buffer[head];
//     }

//     int getState(int tag) {
//         return buffer[tag].state;
//     }

//     int getSize(){
//         return count;
//     }

//     int getOperandReady(int tag) {
//         return buffer[tag].operand_ready;
//     }

//     void printROB() {
//         print_vector(buffer, log_file, "\n");
//     }

//     void removeCompletedEntries()
//     {
//         size_t i = head;
//         size_t elements_checked = 0;

//         while (elements_checked < count) {
//             if (buffer[i] && buffer[i]->completed) {
//                 buffer[i].reset(); // Mark entry as empty
//             }
//             i = (i + 1) % max_size;
//             ++elements_checked;
//         }

//         // Update head to skip empty entries
//         while (count > 0 && buffer[head] == std::nullopt) {
//             head = (head + 1) % max_size;
//             --count;
//         }
//     }

// private:
//     std::vector<ROBEntry> buffer;
//     size_t head;
//     size_t tail;
//     size_t count;
//     const size_t max_size;
// };


class ROB {
public:
    ROB(size_t size) : max_size(size) {}

    bool addInstruction(const ROBEntry& entry) {
        if (isFull()) {
            return false;
        }
        buffer.push_back(entry); // Simply append to the end
        return true;
    }

    bool removeInstruction() {
        if (isEmpty()) {
            return false;
        }
        buffer.erase(buffer.begin()); // Remove the first element
        return true;
    }

    void updateInstruction(const ROBEntry& entry) {
        for (auto& item : buffer) {
            if (item.tag == entry.tag) {
                item = entry;
                return;
            }
        }
    }

    void updateRegisterReadiness(int dest, int compTag) {
        if (dest == -1) return;
        for (auto& item : buffer) {
            if (item.state == STATES["S"] && item.operand_ready == false) {
                if (item.sc1 == compTag) {
                    item.sc1_ready = true;
                }
                if (item.sc2 == compTag) {
                    item.sc2_ready = true;
                }
            }
        }
    }

    void moveToDispatch(vector<ROBEntry> &dispatch_list, const size_t &N, const size_t &cycle) {
        for (auto& item : buffer) {
            if (item.state == STATES["F"] && dispatch_list.size() < 2*N) {
                item.state = STATES["D"];

                if(item.if_cycle_duration == -1) {
                    item.if_cycle_duration = 1;
                    item.id_cycle_duration = 1;
                }

                dispatch_list.push_back(item);
            }
        }
    }

    void WakeUp(vector<pair<int, int>> &register_file, vector<ROBEntry> &issue_list) {
        for (auto& item : buffer) {
            if (item.state == STATES["S"] && item.sc1_ready && item.sc2_ready && !item.operand_ready) {
                item.operand_ready = true;
                auto it = find_if(issue_list.begin(), issue_list.end(), [&](const ROBEntry& entry) {
                    return entry.tag == item.tag;
                });
                if (it != issue_list.end()) {
                    *it = item;
                }
            }
        }
    }

    bool isDone() {
        for(auto &item : buffer) {
            if(item.completed != true) return false;
        }
        return true;
    }

    bool isFull() const {
        return buffer.size() == max_size;
    }

    bool isEmpty() const {
        return buffer.empty();
    }

    ROBEntry getHead() {
        return buffer.front();
    }

    int getSize() {
        return buffer.size();
    }

    void printROB() {
        for (const auto& item : buffer) {
            print_vector(buffer, log_file, "\n");
        }
    }

    vector<ROBEntry> getBuffer()
    {
        return buffer;
    }

    void completedEntries() {
        for (auto& entry : buffer) {
            if (entry.completed) {
                // cout << "HI" <<endl;
                cout << outputFormattedROBEntry(entry);
            }
        }
        return;
    }
private:
    std::vector<ROBEntry> buffer;
    const size_t max_size;
};



void Fetch(int &Icount, ROB &reorder_buffer, ifstream &inputFile, vector<ROBEntry> &dispatch_list, int N, const size_t cycle)
{
    string line;
    int dispatch_size = dispatch_list.size();
    while (dispatch_size < 2*N && getline(inputFile, line) ) {
        stringstream ss(line);
        string pc_hex, op, dest_str, sc1_str, sc2_str;

        try {
            ss >> pc_hex >> op >> dest_str >> sc1_str >> sc2_str;
            ROBEntry entry;

            // Cycle info
            entry.if_cycle_start = cycle;
            entry.id_cycle_start = entry.if_cycle_start + 1;

            entry.tag = Icount++;
            entry.pc = stoi(pc_hex, nullptr, 16);
            entry.operation = stoi(op);
            entry.state = STATES["F"];
            entry.dest = stoi(dest_str);
            entry.sc1 = stoi(sc1_str);
            entry.sc2 = stoi(sc2_str);
            entry.sc1_original = stoi(sc1_str);
            entry.src2_original = stoi(sc2_str);
            entry.operand_ready = false;

            if (!reorder_buffer.addInstruction(entry)) return; 
            // cout << "Added entry " << entry.tag << endl;

            dispatch_size++;

            if(DEBUG) log_file << "FETCH: Added instruction with tag " << entry.tag << " to dispatch list.\n";
            // cout << "FETCH: Added instruction with tag " << entry.tag << " to dispatch list.\n";
            // cout << "FETC:\t";
            // printROBEntry(entry);
        } catch (const std::exception& e) {
            if(DEBUG) log_file << "Error processing line: " << e.what() << endl;
            continue;
        }
    }
    return;
}

void Dispatch(vector<ROBEntry> &dispatch_list, vector<ROBEntry> &issue_list, ROB &reorder_buffer, int N,  int S, vector<pair<int, int>> &register_file, const size_t cycle)
{
    reorder_buffer.moveToDispatch(dispatch_list, N, cycle);
    // cout << "Dispatching..." << endl;
    vector<ROBEntry> temp;

    // Filter the `dispatch_list` for entries in the "D" state
    for (const auto &entry : dispatch_list) {
        if (entry.state == STATES["D"]) {
            temp.push_back(entry);
        }
    }

    // Sort entries by tag
    sort(temp.begin(), temp.end(), [](const ROBEntry &a, const ROBEntry &b) {
        return a.tag < b.tag;
    });

    if (temp.empty()) return;

    vector<ROBEntry> to_remove;  // Track entries to be removed from `dispatch_list`

    for (auto &entry : temp) {
        if (issue_list.size() < S) {
            // Mark for removal
            to_remove.push_back(entry);

            entry.state = STATES["S"];

            if (entry.sc1 != -1 && register_file[entry.sc1].first == 0) {
                entry.sc1 = register_file[entry.sc1].second;
                entry.sc1_ready = false;
            }
            if (entry.sc2 != -1 && register_file[entry.sc2].first == 0) {
                entry.sc2 = register_file[entry.sc2].second;
                entry.sc2_ready = false;
            }
            if (entry.sc1_ready && entry.sc2_ready) entry.operand_ready = true;

            if (entry.dest != -1) {
                register_file[entry.dest].first = 0;
                register_file[entry.dest].second = entry.tag;
            }

            // IS cycle info
            entry.is_cycle_start = cycle+1;

            issue_list.push_back(entry);
            reorder_buffer.updateInstruction(entry);

            if(DEBUG) log_file << "DISPATCH: Instruction with tag " << entry.tag << " moved from dispatch list to issue list.\n";
            // cout << "DISPATCH: Instruction with tag " << entry.tag << " moved from dispatch list to issue list.\n";
        }
    }

    // Remove entries from `dispatch_list` after iteration
    dispatch_list.erase(remove_if(dispatch_list.begin(), dispatch_list.end(),
                                  [&to_remove](const ROBEntry &entry) {
                                      return find(to_remove.begin(), to_remove.end(), entry) != to_remove.end();
                                  }),
                        dispatch_list.end());


    
}


void Issue(vector<ROBEntry> &issue_list, vector<ROBEntry> &execute_list, ROB &reorder_buffer, int N, const size_t cycle) {
    // cout << "Issuing..." << endl;
    vector<ROBEntry> temp;

    // Collect entries in "S" state
    for (const auto &entry : issue_list) {
        if (entry.state == STATES["S"]) {
            temp.push_back(entry);
        }
    }

    // Sort entries by tag
    sort(temp.begin(), temp.end(), [](const ROBEntry &a, const ROBEntry &b) {
        return a.tag < b.tag;
    });

    // int execute_size = execute_list.size();
    if (temp.empty()) return;

    vector<ROBEntry> to_remove;  // Track entries to be removed from `issue_list`

    for (auto &entry : temp) {
        if (entry.operand_ready && execute_list.size() < N) {
            // Mark for removal
            to_remove.push_back(entry);

            entry.state = STATES["E"];
            if (entry.operation == 0) {
                entry.execution_cycles = 1;
            } else if (entry.operation == 1) {
                entry.execution_cycles = 2;
            } else if (entry.operation == 2) {
                entry.execution_cycles = 10;
            }

            // EX cycle info
            if(entry.ex_cycle_start == -1) {
                entry.ex_cycle_start = cycle+1;
                entry.is_cycle_duration = cycle - entry.is_cycle_start;
            }


            execute_list.push_back(entry);
            reorder_buffer.updateInstruction(entry);


            if(DEBUG) log_file << "ISSUE: Instruction with tag " << entry.tag << " moved from issue list to execute list.\n";
            // cout << "ISSUE: Instruction with tag " << entry.tag << " moved from issue list to execute list.\n";
        }
    }

    // Remove entries from `issue_list` after iteration
    issue_list.erase(remove_if(issue_list.begin(), issue_list.end(),
                               [&to_remove](const ROBEntry &entry) {
                                   return find(to_remove.begin(), to_remove.end(), entry) != to_remove.end();
                               }),
                     issue_list.end());
}


void Execute(vector<ROBEntry> &execute_list, vector<ROBEntry> &issue_list, vector<pair<int, int>> &register_file, ROB &reorder_buffer, const size_t cycle) {
    // cout << "Executing..." << endl;
    // if (!execute_list.empty()) {
    //     print_vector(execute_list, log_file, "\n");
    // }

    vector<int> to_remove;  // Track indices to be removed

    for (int i = execute_list.size() - 1; i >= 0; --i) {
        ROBEntry &entry = execute_list[i];
        if (entry.state == STATES["E"] && entry.execution_cycles == 0)
        {
            
            entry.state = STATES["W"];
            entry.completed = true;

            // Track EX cycle
            if (entry.ex_cycle_duration == -1) {
                entry.wb_cycle_start = cycle;
                entry.ex_cycle_duration = cycle - entry.ex_cycle_start;
            }

            reorder_buffer.updateInstruction(entry);

            // Validate `dest` before updating `register_file`
            if (entry.dest >= 0 && entry.dest < register_file.size()) {
                register_file[entry.dest].first = 1;
                // if(entry.tag == 1107) cout << "YEWSS" << endl;
                reorder_buffer.updateRegisterReadiness(entry.dest, entry.tag);
            }

            if(DEBUG) log_file << "EXECUTE END: Instruction with tag " << entry.tag << " completed execution.\n";
            // cout << "EXECUTE END: Instruction with tag " << entry.tag << " completed execution.\n";

            to_remove.push_back(i);  // Mark for removal
        } else {
            if (entry.execution_cycles > 0) {
                entry.execution_cycles--;
            }
            reorder_buffer.updateInstruction(entry);
        }
    }

    // Remove entries from `execute_list` after iteration
    for (int index : to_remove) {
        execute_list.erase(execute_list.begin() + index);
    }

    reorder_buffer.WakeUp(register_file, issue_list);
}


// void Retire(ROB &reorder_buffer, const size_t cycle)
// {
//     while (!reorder_buffer.isEmpty() && reorder_buffer.getHead().completed) {
//         reorder_buffer.retireInstruction(cycle);

//         if(DEBUG) log_file << "RETIRE: Instruction with tag " << reorder_buffer.getHead().tag << " retired.\n";
      
//         // cout << "RETIRE: Instruction with tag " << reorder_buffer.getHead().tag << " retired.\n";
//         reorder_buffer.removeInstruction();
//         if(DEBUG) log_file << "HEAD: " << reorder_buffer.getHead().tag << endl;
//     }
// }


void Retire(ROB &reorder_buffer, const size_t cycle)
{
    // for(auto &item : reorder_buffer.getBuffer()) {
    //     if(item.completed) {
    //         if(DEBUG) log_file << "Retired instruction with tag " << item.tag << endl;
    //     }
    // }
    
}


bool Advance_Cycle(ROB &reorder_buffer, vector<ROBEntry> &dispatch_list, const size_t &N, size_t &cycle)
{
    if (reorder_buffer.isDone())
    {
        // cout << "No instructions to advance" << endl;
        return false;
    }
    // cout << "Advancing cycle..." << endl;

    cycle++;
    return true;
}


int main(int argc, char** argv)
{
    if(DEBUG) log_file << "HI" << endl;
    size_t N = stoi(argv[1]);
    size_t S = stoi(argv[2]);

    string trace_file = argv[3];

    ifstream inputFile(trace_file);
    if(!inputFile)
    {
        cout << "Error opening file" << endl;
        return 0;
    }

    ROB reorder_buffer = ROB(1000*S);

    vector<ROBEntry> dispatch_list;
    vector<ROBEntry> issue_list;
    vector<ROBEntry> execute_list;
    vector<pair<int, int>> register_file(128, {1, -1});

    int Icount = 0;
    size_t cycle = 0;
    do
    {
        // cout << "Cycle " << cycle << endl;
        // cout << "ROB size " << reorder_buffer.getSize() << endl;
        Retire(reorder_buffer, cycle);
        Execute(execute_list, issue_list, register_file, reorder_buffer, cycle);
        Issue(issue_list, execute_list, reorder_buffer, N, cycle);
        Dispatch(dispatch_list, issue_list, reorder_buffer, N, S, register_file, cycle);
        Fetch(Icount, reorder_buffer, inputFile, dispatch_list, N, cycle);

        // log_file << "Issue list: " << endl;
        // print_vector(issue_list, log_file, "\n");

        // reorder_buffer.printROB();

        // log_file << "Execute list: " << endl;
        // print_vector(execute_list, log_file, "\n");
        // if(Icount < 40) reorder_buffer.print_contents();
    }while(Advance_Cycle(reorder_buffer, dispatch_list, N, cycle));
 
    reorder_buffer.completedEntries();

    return 0;
}
