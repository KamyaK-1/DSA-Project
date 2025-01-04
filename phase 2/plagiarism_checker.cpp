#include "plagiarism_checker.hpp"

// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class
plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> & __submissions)
{
    auto current_time = std::chrono::steady_clock::now();
    auto base_timestamp = current_time - std::chrono::minutes(10);

    for (int i = 0; i < __submissions.size(); i++){
        submissions.push_back(make_pair(base_timestamp, __submissions[i])); // clock time
        calculate_hashes(__submissions[i]);
    }
    initial_length = submissions.size();
    already_plagged.resize(__submissions.size(), false); 
    is_adding_submission = false;
    finished = false;
    evaluate_submissions =  std::thread(&plagiarism_checker_t::start_plag_checking,this);
}
plagiarism_checker_t::plagiarism_checker_t(void)
{
    initial_length = 0;
    is_adding_submission = false;
    finished = false;
    evaluate_submissions =  std::thread(&plagiarism_checker_t::start_plag_checking,this);
}
plagiarism_checker_t::~plagiarism_checker_t(void)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    if (evaluate_submissions.joinable())
        evaluate_submissions.join();
    // assert(false);
}


void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission){
    // is_adding_submission = true;
    if (__submission == nullptr) 
        return;
    std::chrono::steady_clock::time_point x = std::chrono::steady_clock::now();
    // std::thread queue_the_submission(&plagiarism_checker_t::queue_submission, this, x, __submission);
    // queue_the_submission.detach();
    std::lock_guard<std::mutex> lock(mtx);
    queued_submissions.push({x, __submission});
    // already_plagged.push_back(false);
    // is_adding_submission = false;
    // cv.notify_all();
}

// void plagiarism_checker_t::queue_submission(std::chrono::steady_clock::time_point x, std::shared_ptr<submission_t> __submission){
//     std::lock_guard<std::mutex> lock(mtx);
//     queued_submissions.push({x, __submission});
// }

int inv(int a , int m) {
  return a <= 1 ? a : (m - ((long long)(m/a) * inv(m % a , m) % m) + m) % m;
}

long long binary_exp(int a, int power, int modulo)
{
    if (power == 0)
        return 1LL;
    long long t = binary_exp(a,power >> 1, modulo);
    t = t * t % modulo;
    if ((power & 1) == 0)
        return t;
    return (t * a)%modulo;
}
void rolling_hash(std::vector<int> &tokenized_codefile, std::vector<unsigned int> &hashes , int len)
{
    if(len >= tokenized_codefile.size())
    {
        return;
    }
    hashes.resize(tokenized_codefile.size() + 1 - len);
    const int prime = 31;
    const int modulo = 1000000007;
    
    long long hash = 0;
    long long power = binary_exp(prime,len-1,modulo);
    for (int i = 0 ; i < len ; i++)
    {
        hash = (hash * prime + tokenized_codefile[i]) % modulo;
    }
    hashes[0] = hash;
    for (int index_start = 1; index_start <= tokenized_codefile.size() - len;  index_start++)
    {
        hash = ((hash - tokenized_codefile[index_start-1] * power)%modulo + modulo) % modulo;
        hash = (hash * prime + tokenized_codefile[index_start - 1 + len]) % modulo;
        hashes[index_start]=hash;
}
}

void plagiarism_checker_t::calculate_hashes(std::shared_ptr<submission_t> __submission){
    tokenizer_t T(__submission->codefile);
    std::vector<int> tokenized_codefile = T.get_tokens();

    std::vector<unsigned int> hashes_small , hashes_large;
    rolling_hash(tokenized_codefile,hashes_small,15);
    rolling_hash(tokenized_codefile,hashes_large,75);

    hashes_15_unsorted[__submission] = hashes_small;
    std::sort(hashes_small.begin(),hashes_small.end());
    std::sort(hashes_large.begin(),hashes_large.end());
    hashes_15[__submission] = hashes_small;
    hashes_75[__submission] = hashes_large;   
}


void plagiarism_checker_t::call_plag(std::shared_ptr<submission_t> __submission)
{
    if (__submission->student != nullptr)
        __submission->student->flag_student(__submission);
    if (__submission->professor != nullptr)
        __submission->professor->flag_professor(__submission);
   
}


void plagiarism_checker_t::start_plag_checking(void){
    int evaluation_ptr = initial_length;
    std::vector<unsigned int> previous_submission;
    while(true)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!queued_submissions.empty()){
                submissions.push_back(queued_submissions.front());
                queued_submissions.pop();
                already_plagged.push_back(false);
            }
        }
        
        {   
            std::lock_guard<std::mutex> lock(mtx);
            if (finished && evaluation_ptr == submissions.size())
            {
                
                break;
            }
        }
        if (evaluation_ptr < submissions.size())
        {
            if (hashes_15.find(submissions[evaluation_ptr].second) == hashes_15.end())
            {
                calculate_hashes(submissions[evaluation_ptr].second);
            }
            // calculate_hashes(submissions[evaluation_ptr].second);

            auto hashes_small = hashes_15_unsorted[submissions[evaluation_ptr].second];
            auto hashes_large = hashes_75[submissions[evaluation_ptr].second];

            std::set<int> small_matches_indices;

            for (int file_num = evaluation_ptr - 1 ; file_num >= 0; file_num--){
                std::chrono::duration<double> elapsed_seconds = submissions[evaluation_ptr].first - submissions[file_num].first;
                if (elapsed_seconds.count() > 1 && already_plagged[evaluation_ptr])
                {
                    break;
                }
                int counter = 0;
                bool plag = false;
                previous_submission =  hashes_15[submissions[file_num].second]; // search in this array
                for (int hash_num = 0 ; hash_num < hashes_small.size() && counter < 10;)
                {
                    // binary search in the sorted array in previous submission
                    if (std::binary_search(previous_submission.begin(),previous_submission.end(),hashes_small[hash_num]))
                    {
                        small_matches_indices.insert(hash_num/15);
                        hash_num += 15;
                        counter++;   
                    }
                    else
                    {
                        hash_num++;
                    }
                }
                if (counter >= 10 )
                {
                    plag = true;
                }
                else
                {
                    previous_submission = hashes_75[submissions[file_num].second]; // 75 length array .. assuming it it sorted
                    
                    int first_index = 0 , second_index = 0;
                    while (first_index < previous_submission.size() && second_index < hashes_large.size())
                    {
                        if (previous_submission[first_index] == hashes_large[second_index])
                        {
                            plag = true;
                            break;
                        }
                        else if (previous_submission[first_index] < hashes_large[second_index])
                        {
                            first_index++;
                        }
                        else
                        {
                            second_index++;
                        }
                    }
    
                }
                if (plag)
                {
                    // flag kar dena yyad se
                    already_plagged[evaluation_ptr] = true;
                    elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(submissions[evaluation_ptr].first - submissions[file_num].first);
                    if (elapsed_seconds.count() < 1)
                    {   
                        if (!already_plagged[file_num])
                        {
                            // flag this file;
                            if (submissions[file_num].second->student->get_name() == "HermioneGranger")
                            {
                                std::cerr << "flagged " << evaluation_ptr << " \t \t \t \t \t \t " << file_num << " \n\n\n ";
                                std::cerr << submissions[file_num].second->codefile << " \n\n\n ";
                                std::cerr << submissions[evaluation_ptr].second->codefile << " \n\n\n ";
                            }
                            call_plag(submissions[file_num].second);
                            already_plagged[file_num] = true;
                        }
                    }
                }
            }

            if (already_plagged[evaluation_ptr] || small_matches_indices.size() >= 20)
            {
                call_plag(submissions[evaluation_ptr].second);
                already_plagged[evaluation_ptr] = true;
                evaluation_ptr++;
                continue;
            }
            

            bool escaped = false;
            int previous_end = evaluation_ptr + 1;
            for (int file_num = evaluation_ptr + 1 ; file_num < submissions.size(); file_num++)
            {
                if (hashes_15.find(submissions[file_num].second) == hashes_15.end())
                {
                    calculate_hashes(submissions[file_num].second);
                }
                previous_end = file_num + 1;
                std::chrono::duration<double> elapsed_seconds = submissions[evaluation_ptr].first - submissions[file_num].first;
                if (elapsed_seconds.count() > 1)
                {
                    escaped = true;
                    break;
                }
               
                previous_submission =  hashes_15[submissions[file_num].second];
                for (int hash_num = 0 ; hash_num < hashes_small.size();)
                {
                    // binary search in the sorted array in previous submission
                    if (std::binary_search(previous_submission.begin(),previous_submission.end(),hashes_small[hash_num]))
                    {
                        small_matches_indices.insert(hash_num/15);
                        hash_num += 15;
                    }
                    else
                    {
                        hash_num++;
                    }
                }
                if (small_matches_indices.size() >= 20)
                {
                    already_plagged[evaluation_ptr] = true;
                    break;
                }
            }
            if (already_plagged[evaluation_ptr])
            {
                call_plag(submissions[evaluation_ptr].second);
                evaluation_ptr++;
                continue;
            }

            if (escaped)
            {
                evaluation_ptr++;
                continue;
            }
            auto t = submissions[evaluation_ptr].first + std::chrono::seconds(1);
            std::this_thread::sleep_until(t);

            // std::unique_lock<std::mutex> lock(mtx);
            // cv.wait(lock, [this] {return !is_adding_submission.load();});
            {
                std::lock_guard<std::mutex> lock(mtx);
                while(!queued_submissions.empty()){
                    submissions.push_back(queued_submissions.front());
                    queued_submissions.pop();
                    already_plagged.push_back(false);
                }
            }

            for (int file_num = previous_end ; file_num < submissions.size() ; file_num++)
            {
                if (hashes_15.find(submissions[file_num].second) == hashes_15.end())
                {
                    calculate_hashes(submissions[file_num].second);
                }
                std::chrono::duration<double> elapsed_seconds = submissions[evaluation_ptr].first - submissions[file_num].first;
                if (elapsed_seconds.count() > 1)
                {
                    escaped = true;
                    break;
                }
                previous_submission =  hashes_15[submissions[file_num].second];
                for (int hash_num = 0 ; hash_num < hashes_small.size();)
                {
                    // binary search in the sorted array in previous submission
                    if (std::binary_search(previous_submission.begin(),previous_submission.end(),hashes_small[hash_num]))
                    {
                        small_matches_indices.insert(hash_num/15);
                        hash_num += 15;
                    }
                    else
                    {
                        hash_num++;
                    }
                }
                if (small_matches_indices.size() >= 20)
                {
                    already_plagged[evaluation_ptr] = true;
                    break;
                }                
            }
            if (already_plagged[evaluation_ptr])
            {
                call_plag(submissions[evaluation_ptr].second);
            }
            evaluation_ptr++;
            
        }           
    }
}