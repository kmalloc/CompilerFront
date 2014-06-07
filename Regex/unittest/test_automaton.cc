#include "gtest/gtest.h"

#include <map>
#include <string>
#include <sstream>
#include <climits>
#include <sstream>
#include <algorithm>

#define private public
#define protected public

#include "RegExpAutomata.h"
#include "RegExpSyntaxTree.h"

class nfa_case
{
    public:

        nfa_case(const char* pattern, bool partial = true)
            :pattern_(pattern), nfa_(partial), tree_(), txt2match_()
        {
            tree_.BuildSyntaxTree(pattern, pattern + strlen(pattern) - 1);
            nfa_.BuildMachine(&tree_);
        }

        void AddTestCase(const std::string& txt, bool ismatch)
        {
            txt2match_[txt] = ismatch;
        }

        bool IsMatch(const std::string& txt) { return txt2match_[txt]; }

    private:

        std::string pattern_;
        RegExpNFA nfa_;
        RegExpSyntaxTree tree_;
        std::map<std::string, bool> txt2match_;
};

TEST(test_matching_txt, test_automata_gen)
{
    std::vector<nfa_case*> cases;

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    nfa_case* b1_0 = new nfa_case("a(bc)(\\0df)(g\\1)e", false);
    b1_0->AddTestCase("abcbcdfgbcdfe", true);
    b1_0->AddTestCase("abcbedfgbcdfe", false);
    b1_0->AddTestCase("abcbcdfbcdfe", false);
    cases.push_back(b1_0);

    nfa_case* b1_1 = new nfa_case("a(bc)(\\0ef)*v", false);
    b1_1->AddTestCase("abcbcefbcefv", true);
    b1_1->AddTestCase("abcbcefbccefv", false);
    b1_1->AddTestCase("abcv", true);
    cases.push_back(b1_1);

    nfa_case* b1_2 = new nfa_case("a(bc)(ef\\0*)v", false);
    b1_2->AddTestCase("abcefv", true);
    b1_2->AddTestCase("abcefbcv", true);
    b1_2->AddTestCase("abcefbcbcv", true);
    b1_2->AddTestCase("abcv", false);
    cases.push_back(b1_2);

    nfa_case* b1_3 = new nfa_case("a(bc)(ef\\0)\\1v\\1", false);
    b1_3->AddTestCase("abcefbcefbcvefbc", true);
    b1_3->AddTestCase("abcbcefbccefv", false);
    b1_3->AddTestCase("abcv", false);
    cases.push_back(b1_3);

    nfa_case* b1_4 = new nfa_case("(ming|dong)\\0", false);
    b1_4->AddTestCase("mingming", true);
    b1_4->AddTestCase("dongdong", true);
    b1_4->AddTestCase("mingdong", false);
    b1_4->AddTestCase("dongming", false);
    cases.push_back(b1_4);

    nfa_case* c8_3 = new nfa_case("(a*)*", false);
    c8_3->AddTestCase("a", true);
    c8_3->AddTestCase("aa", true);
    c8_3->AddTestCase("aaa", true);
    c8_3->AddTestCase("ab", false);
    c8_3->AddTestCase("bb", false);
    cases.push_back(c8_3);

    nfa_case* c8_4 = new nfa_case("(a*)*bc\\0", false);
    c8_4->AddTestCase("abca", true);
    c8_4->AddTestCase("aabcaa", false);
    c8_4->AddTestCase("aabca", true);
    c8_4->AddTestCase("abc", false);
    c8_4->AddTestCase("bbc", false);
    c8_4->AddTestCase("bc", true);
    cases.push_back(c8_4);

    nfa_case* c8_5 = new nfa_case("a(bc)*fe\\0", false);
    c8_5->AddTestCase("afe", true);
    c8_5->AddTestCase("abcfebc", true);
    c8_5->AddTestCase("abcbcfebc", true);
    c8_5->AddTestCase("afefe", false);
    cases.push_back(c8_5);

    nfa_case* c8_6 = new nfa_case("(a*)bc\\0", false);
    c8_6->AddTestCase("aabca", false);
    c8_6->AddTestCase("aabcaa", true);
    c8_6->AddTestCase("bc", true);
    cases.push_back(c8_6);

    nfa_case* c8_0 = new nfa_case("(((ab)))\\0\\1\\2", false);
    c8_0->AddTestCase("abababab", true);
    c8_0->AddTestCase("ababab", false);
    c8_0->AddTestCase("ababcdab", false);
    cases.push_back(c8_0);

    nfa_case* c8_1 = new nfa_case("a([bc])(cd)\\0\\1", false);
    c8_1->AddTestCase("abcdbcd", true);
    c8_1->AddTestCase("abcdcd", false);
    c8_1->AddTestCase("abccd", false);
    c8_1->AddTestCase("abd", false);
    cases.push_back(c8_1);

    nfa_case* c8 = new nfa_case("a([bc]*)(c*d)\\0", false);
    c8->AddTestCase("abcdbc", true);
    c8->AddTestCase("abccdbcc", true);
    c8->AddTestCase("cde", false);
    c8->AddTestCase("ab", false);
    c8->AddTestCase("abdb", true);
    cases.push_back(c8);

    nfa_case* c8_2 = new nfa_case("a([bc]*)(c*d)\\0\\1vef", false);
    c8_2->AddTestCase("abcdbcdvef", true);
    c8_2->AddTestCase("abccdbccdvef", true);
    c8_2->AddTestCase("abdbdvef", false);
    c8_2->AddTestCase("abb", false);
    c8_2->AddTestCase("abdbdvef", true);
    cases.push_back(c8_2);

    nfa_case* cg = new nfa_case("(a)(b)(cd)\\0\\2\\1", false);
    cg->AddTestCase("abcdacdb", true);
    cg->AddTestCase("acb", false);
    cg->AddTestCase("abcd", false);
    cases.push_back(cg);

    nfa_case* cg1 = new nfa_case("(((ab)c)e)\\0\\1\\2", false);
    cg1->AddTestCase("abceababcabce", true);
    cg1->AddTestCase("abceabcababce", false);
    cg1->AddTestCase("abceabcabceab", false);
    cg1->AddTestCase("abce", false);
    cg1->AddTestCase("abceab", false);
    cases.push_back(cg1);

    nfa_case* cg2 = new nfa_case("(a(b(cd)))\\2\\1\\0", false);
    cg2->AddTestCase("abcdcdbcdabcd", false);
    cg2->AddTestCase("abcdabcdbcdcd", true);
    cg2->AddTestCase("abcd", false);
    cg2->AddTestCase("abcdcd", false);
    cg2->AddTestCase("abcdcdb", false);
    cases.push_back(cg2);

    nfa_case* cg3 = new nfa_case("(ab|cd)efv\\0", false);
    cg3->AddTestCase("abefvab", true);
    cg3->AddTestCase("cdefvcd", true);
    cg3->AddTestCase("abefvcd", false);
    cases.push_back(cg3);

    nfa_case* cg4 = new nfa_case("((ab|nm)|cd)efv\\0\\1", false);
    cg4->AddTestCase("abefvabab", true);
    cg4->AddTestCase("nmefvnmnm", true);
    cg4->AddTestCase("abefvab", false);
    cases.push_back(cg4);

    nfa_case* cg5 = new nfa_case("((ab|nm)|(gh|cd))efv\\0\\1", false);
    cg5->AddTestCase("abefvabab", true);
    cg5->AddTestCase("nmefvnmnm", true);
    cg5->AddTestCase("ghefvghgh", true);
    cg5->AddTestCase("cdefvcdcd", true);
    cg5->AddTestCase("cdefvghcd", false);
    cg5->AddTestCase("cdefvghgh", false);
    cg5->AddTestCase("nmefvabnm", false);
    cases.push_back(cg5);

    nfa_case* cg6 = new nfa_case("((ab)*ef(gnw)((vm)*))tu\\0\\1\\2\\3g\\4\\2end", false);
    cg6->AddTestCase("abefgnwvmvmtuabgnwvmvmvmgabefgnwvmvmvmend", true);
    cg6->AddTestCase("abababefgnwvmvmtuabgnwvmvmvmgabababefgnwvmvmvmend", true);
    cg6->AddTestCase("abababefgnwvmvmtuabgnwvmvmvmgababefgnwvmvmvmend", false);
    cg6->AddTestCase("efgnwtugnwgefgnwend", true);
    cg6->AddTestCase("efgnwtugnwgefgnwefend", false);
    cases.push_back(cg6);

    nfa_case* c18_0 = new nfa_case("(ab){2, 4}\\0", false);
    c18_0->AddTestCase("ababababab", true);
    c18_0->AddTestCase("ababab", true);
    c18_0->AddTestCase("abababababababab", false);
    cases.push_back(c18_0);

    nfa_case* c18_1 = new nfa_case("(ab){2,4}ce\\0mn", false);
    c18_1->AddTestCase("ababababceabmn", true);
    c18_1->AddTestCase("ababceabmn", true);
    c18_1->AddTestCase("abababababceababceabmn", false);
    cases.push_back(c18_1);

    nfa_case* c18_2 = new nfa_case("((ab)*de)fn\\0\\1", false);
    c18_2->AddTestCase("ababdefnabab", false);
    c18_2->AddTestCase("ababdefnababab", false);
    c18_2->AddTestCase("ababdefnababde", false);
    c18_2->AddTestCase("ababdefnabababde", true);
    c18_2->AddTestCase("defnde", true);
    c18_2->AddTestCase("defn", false);
    cases.push_back(c18_2);

    nfa_case* c18_3 = new nfa_case("(ab(cd)*ef)gn\\0\\1", false);
    c18_3->AddTestCase("abcdcdefgncdcdcdef", false);
    c18_3->AddTestCase("abcdcdefgncdabcdcdef", true);
    c18_3->AddTestCase("abcdcdefgncdcdef", false);
    c18_3->AddTestCase("abcdcdefgncdef", false);
    c18_3->AddTestCase("abefgnef", false);
    c18_3->AddTestCase("abefgnabef", true);
    c18_3->AddTestCase("abefgnefef", false);
    cases.push_back(c18_3);

    nfa_case* c18_4 = new nfa_case("(ab(cd)*)gn\\0\\1", false);
    c18_4->AddTestCase("abcdcdgncdabcdcd", true);
    c18_4->AddTestCase("abcdcdgncdcdabcdcd", false);
    c18_4->AddTestCase("abcdgncdabcd", true);
    c18_4->AddTestCase("abcdgncd", false);
    c18_4->AddTestCase("abcdgnab", false);
    cases.push_back(c18_4);

    nfa_case* c18_5 = new nfa_case("((ab)*)cd\\0\\1", false);
    c18_5->AddTestCase("ababcdababab", true);
    c18_5->AddTestCase("ababcdabab", false);
    cases.push_back(c18_5);

    nfa_case* c18_6 = new nfa_case("(ab)*cd\\0", false);
    c18_6->AddTestCase("ababcdababab", false);
    c18_6->AddTestCase("ababcdabab", false);
    c18_6->AddTestCase("ababcdab", true);
    c18_6->AddTestCase("ababcd", false);
    cases.push_back(c18_6);

#endif

    nfa_case* c1 = new nfa_case("^([abc]+\\d)*(a|b)+3\\w2e");
    c1->AddTestCase("a3b3c2e", true);
    c1->AddTestCase("aa3b3c2e", true);
    c1->AddTestCase("ab3b3c2e", true);
    c1->AddTestCase("ab32ab3e2e", false);
    c1->AddTestCase("ab3ac32e", false);
    c1->AddTestCase("ab3ab3aa3e2e", true);
    c1->AddTestCase("ab3aa3aa3v2e", true);
    c1->AddTestCase("ab32ab32e", false);
    c1->AddTestCase("ab3b4c2a3e", false);
    cases.push_back(c1);

    nfa_case* c2 = new nfa_case("(abc)+\\d((ev){2,5})?$");
    c2->AddTestCase("abc3", true);
    c2->AddTestCase("abc3evevev", true);
    c2->AddTestCase("abc3evevevevevev", false);
    c2->AddTestCase("abcabcabc3", true);
    c2->AddTestCase("abcara3", false);
    c2->AddTestCase("abcabbaae2", false);
    cases.push_back(c2);

    nfa_case* c3 = new nfa_case("regexp|coding", false);
    c3->AddTestCase("regexp", true);
    c3->AddTestCase("sregexp", false);
    c3->AddTestCase("coding", true);
    c3->AddTestCase("codingv", false);
    c3->AddTestCase("\\|", false);
    cases.push_back(c3);

    nfa_case* c3_0 = new nfa_case("(regexp|coding)", false);
    c3_0->AddTestCase("regexp", true);
    c3_0->AddTestCase("coding", true);
    c3_0->AddTestCase("codingv", false);
    c3_0->AddTestCase("\\|", false);
    cases.push_back(c3_0);

    nfa_case* c3_1 = new nfa_case("(regexp|(coding))", false);
    c3_1->AddTestCase("regexp", true);
    c3_1->AddTestCase("coding", true);
    c3_1->AddTestCase("codingv", false);
    c3_1->AddTestCase("\\|", false);
    cases.push_back(c3_1);

    nfa_case* c4 = new nfa_case("([abcdef][0123456]+,)+", false);
    c4->AddTestCase("a33", false);
    c4->AddTestCase("a33,", true);
    c4->AddTestCase("a2,a3,b4", false);
    c4->AddTestCase("a2,a3,", true);
    c4->AddTestCase("a332,b3,b34,", true);
    c4->AddTestCase("aa332,b3,b34,", false);
    c4->AddTestCase("aa332,bb3,b34,", false);
    c4->AddTestCase("a332,bb3,b34,", false);
    cases.push_back(c4);

    nfa_case* c5 = new nfa_case(".*regexp.*", false);
    c5->AddTestCase("regexp", true);
    c5->AddTestCase("aaregexp", true);
    c5->AddTestCase("regexpbb", true);
    c5->AddTestCase("aaregexpbb", true);
    c5->AddTestCase("aaraegexpbb", false);
    c5->AddTestCase("aaregexpregexpbb", true);
    c5->AddTestCase("aaregsexpregexpbb", true);
    c5->AddTestCase("aaregesxpxpbb", false);
    cases.push_back(c5);

    nfa_case* c6 = new nfa_case("abc", false);
    c6->AddTestCase("", false);
    cases.push_back(c6);

    nfa_case* c7 = new nfa_case("^(ab|cd)e");
    c7->AddTestCase("abcde", false);
    c7->AddTestCase("cde", true);
    c7->AddTestCase("abe", true);
    cases.push_back(c7);

    nfa_case* c7_0 = new nfa_case("(ab|cd)e", false);
    c7_0->AddTestCase("abcde", false);
    c7_0->AddTestCase("cde", true);
    c7_0->AddTestCase("cvde", false);
    c7_0->AddTestCase("abe", true);
    cases.push_back(c7_0);

    nfa_case* c8_00 = new nfa_case("a([bc]+)(c*d)", false);
    c8_00->AddTestCase("abcd", true);
    c8_00->AddTestCase("abcbccd", true);
    c8_00->AddTestCase("abccd", true);
    c8_00->AddTestCase("cde", false);
    c8_00->AddTestCase("abc", false);
    c8_00->AddTestCase("abd", true);
    cases.push_back(c8_00);

    nfa_case* c9 = new nfa_case("(ab|a)b*c", false);
    c9->AddTestCase("abc", true);
    c9->AddTestCase("abbc", true);
    c9->AddTestCase("cde", false);
    c9->AddTestCase("ab", false);
    c9->AddTestCase("abbc", true);
    cases.push_back(c9);

    nfa_case* c10 = new nfa_case("((a)(b)c)(d)", false);
    c10->AddTestCase("abcd", true);
    c10->AddTestCase("cde", false);
    c10->AddTestCase("ab", false);
    c10->AddTestCase("abc", false);
    cases.push_back(c10);

    nfa_case* c11 = new nfa_case("^(ab|ab+)ef");
    c11->AddTestCase("abef", true);
    c11->AddTestCase("abbbbef", true);
    c11->AddTestCase("ababef", false);
    cases.push_back(c11);

    nfa_case* c12 = new nfa_case("^(a(bc+|b[eh])g|.h)$");
    c12->AddTestCase("bbabhg", false);
    c12->AddTestCase("abh", false);
    c12->AddTestCase("babh", false);
    c12->AddTestCase("abccg", true);
    c12->AddTestCase("abeg", true);
    c12->AddTestCase("gh", true);
    cases.push_back(c12);

    nfa_case* c13 = new nfa_case("1addtest vsl(i)e lsdjfsjfsd efljklsee l(vvwwef) \\w\\d kjsdf.*[abc]dwef\
            for &&wwwevv122455 // vvdwfff ewwwevf fssdfs asdfs ldfjskldfjs 43953485wewew@#@#\\$%\
            mapvector lsjfsdf 323rfdslfs ljfs{3,5}");
    c13->AddTestCase("addtest", false);
    cases.push_back(c13);

    nfa_case* c14 = new nfa_case("abc[vbe-i]yy", false);
    c14->AddTestCase("abcvyy", true);
    c14->AddTestCase("abcbyy", true);
    c14->AddTestCase("abceyy", true);
    c14->AddTestCase("abcfyy", true);
    c14->AddTestCase("abcgyy", true);
    c14->AddTestCase("abchyy", true);
    c14->AddTestCase("abciyy", true);
    c14->AddTestCase("abcnyy", false);
    c14->AddTestCase("abclyy", false);
    cases.push_back(c14);

    nfa_case* c14_0 = new nfa_case("ab[va-b]c", false);
    c14_0->AddTestCase("abvc", true);
    c14_0->AddTestCase("abac", true);
    c14_0->AddTestCase("abbc", true);
    c14_0->AddTestCase("abec", false);
    cases.push_back(c14_0);

    nfa_case* c14_1 = new nfa_case("ab[vb-b]c", false);
    c14_1->AddTestCase("abbc", true);
    c14_1->AddTestCase("abec", false);
    cases.push_back(c14_1);

    nfa_case* c14_2 = new nfa_case("ab[-vb-f]c", false);
    c14_2->AddTestCase("abbc", true);
    c14_2->AddTestCase("ab-c", true);
    c14_2->AddTestCase("abec", true);
    c14_2->AddTestCase("abmc", false);
    c14_2->AddTestCase("abpc", false);
    cases.push_back(c14_2);

    nfa_case* c14_3 = new nfa_case("ab[vb-f\\-]c", false);
    c14_3->AddTestCase("abbc", true);
    c14_3->AddTestCase("ab-c", true);
    c14_3->AddTestCase("ab\\c", true);
    c14_3->AddTestCase("abec", true);
    c14_3->AddTestCase("abmc", false);
    c14_3->AddTestCase("abpc", false);
    cases.push_back(c14_3);

    nfa_case* c14_4 = new nfa_case("ab[ab\\-f]c", false);
    c14_4->AddTestCase("abac", true);
    c14_4->AddTestCase("abbc", true);
    c14_4->AddTestCase("ab-c", true);
    c14_4->AddTestCase("abfc", true);
    c14_4->AddTestCase("ab\\c", false);
    c14_4->AddTestCase("abcc", false);
    c14_4->AddTestCase("abec", false);
    cases.push_back(c14_4);

    nfa_case* c15 = new nfa_case("ab[^qwerty]vn", false);
    c15->AddTestCase("ab[vn", true);
    c15->AddTestCase("ab]vn", true);
    c15->AddTestCase("ab^vn", true);
    c15->AddTestCase("abavn", true);
    c15->AddTestCase("abbvn", true);
    c15->AddTestCase("abuvn", true);
    c15->AddTestCase("abgvn", true);
    c15->AddTestCase("abqvn", false);
    c15->AddTestCase("abwvn", false);
    c15->AddTestCase("abevn", false);
    c15->AddTestCase("abrvn", false);
    c15->AddTestCase("abtvn", false);
    c15->AddTestCase("abyvn", false);
    cases.push_back(c15);

    nfa_case* c15_0 = new nfa_case("ab[^qwa-d\\-p]vn", false);
    c15_0->AddTestCase("ab[vn", true);
    c15_0->AddTestCase("ab]vn", true);
    c15_0->AddTestCase("abuvn", true);
    c15_0->AddTestCase("abgvn", true);
    c15_0->AddTestCase("ab-vn", false);
    c15_0->AddTestCase("abpvn", false);
    c15_0->AddTestCase("abqvn", false);
    c15_0->AddTestCase("abwvn", false);
    c15_0->AddTestCase("abavn", false);
    c15_0->AddTestCase("abbvn", false);
    c15_0->AddTestCase("abcvn", false);
    c15_0->AddTestCase("abdvn", false);
    cases.push_back(c15_0);

    nfa_case* c16 = new nfa_case("ab[vb^n\\g\\-h]my", false);
    c16->AddTestCase("abvmy", true);
    c16->AddTestCase("abbmy", true);
    c16->AddTestCase("abnmy", true);
    c16->AddTestCase("ab\\my", true);
    c16->AddTestCase("abgmy", true);
    c16->AddTestCase("ab-my", true);
    c16->AddTestCase("abhmy", true);
    c16->AddTestCase("ab^my", true);
    cases.push_back(c16);

    nfa_case* c17 = new nfa_case("(ab){0, 4}cd", false);
    c17->AddTestCase("cd", true);
    c17->AddTestCase("abcd", true);
    c17->AddTestCase("ababcd", true);
    c17->AddTestCase("abababcd", true);
    c17->AddTestCase("ababababcd", true);
    c17->AddTestCase("abababababcd", false);
    c17->AddTestCase("ababababababcd", false);
    cases.push_back(c17);

    nfa_case* c17_0 = new nfa_case("(ev){2,5}", false);
    c17_0->AddTestCase("evevevevev", true);
    c17_0->AddTestCase("evevevev", true);
    c17_0->AddTestCase("evevev", true);
    c17_0->AddTestCase("evev", true);
    c17_0->AddTestCase("ev", false);
    c17_0->AddTestCase("evevevevevev", false);
    cases.push_back(c17_0);

    // match even number 0 and odd number 1
    nfa_case* ct_0 = new nfa_case("((1|0(00)*01)((11|10(00)*01))*|(0(00)*1|(1|0(00)*01)((11|10(00)*01))*(0|10(00)*1))((1(00)*1|(0|1(00)*01)((11|10(00)*01))*(0|10(00)*1)))*(0|1(00)*01)((11|10(00)*01))*)", false);
   ct_0->AddTestCase("0111000", true);
   ct_0->AddTestCase("01110000", false);
   ct_0->AddTestCase("000011111", true);
   ct_0->AddTestCase("0000111111", false);
   ct_0->AddTestCase("0000011111", false);
   ct_0->AddTestCase("00000111110", true);
   cases.push_back(ct_0);

   nfa_case* ct_1 = new nfa_case("M{0,4}(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3})", false);
   ct_1->AddTestCase("CMXCIV", true);
   ct_1->AddTestCase("MMCMXCIV", true);
   ct_1->AddTestCase("MMCMDXCIV", false);
   ct_1->AddTestCase("MMDCXCIV", true);
   ct_1->AddTestCase("MMDCCCXCIV", true);
   ct_1->AddTestCase("MMMMCMXCIV", true);
   ct_1->AddTestCase("MMMMMCMXCIV", false);
   cases.push_back(ct_1);

    for (size_t i = 0; i < cases.size(); ++i)
    {
        for (std::map<std::string, bool>::iterator it = cases[i]->txt2match_.begin();
                it != cases[i]->txt2match_.end(); ++it)
        {
            try
            {
                EXPECT_EQ(it->second, cases[i]->nfa_.RunMachine(it->first.c_str(), it->first.c_str() + it->first.size() - 1))
                    << "case:" << i << ", pattern:" << cases[i]->pattern_ << ", test:" << it->first << std::endl;
            }
            catch (...)
            {
                std::cout << "exception occurs." << std::endl
                    << "case:" << i << ", pattern:" << cases[i]->pattern_ << ", test:" << it->first << std::endl;
            }
        }

        delete cases[i];
    }
}

