//
// password_rating.cpp - password rating implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui.h"
#include "dictionary/dictionary.h"

#define MAX_STR	256
#define MAX_LCS 256	// Maximum size of the longest common sequence
#define NO_OF_CHARS 256

/// <summary>
/// Function removes duplicate characters from the string
/// This function work in-place and fills null characters
/// in the extra space left
/// </summary>
std::string remove_duplicates(const std::string& str_in) {
	char str[NO_OF_CHARS];
	strncpy_s(str, str_in.c_str(), sizeof(str));

	int bin_hash[NO_OF_CHARS] = { 0 };
	int ip_ind = 0, res_ind = 0;
	char temp;

	// In place removal of duplicate characters
	while (*(str + ip_ind)) {
		temp = *(str + ip_ind);
		if (bin_hash[temp] == 0)
		{
			bin_hash[temp] = 1;
			*(str + res_ind) = *(str + ip_ind);
			res_ind++;
		}
		ip_ind++;
	}

	/// After above step string is stringiittg.
	/// Removing extra iittg after string
	*(str + res_ind) = '\0';
	return str;
}

// determine number of duplicated characters in a string
int number_of_duplicates(const std::string& str_in) {
	return int(str_in.length()) - int(remove_duplicates(str_in).length());
}

// Quick and dirty swap of the address of 2 arrays of unsigned int
void swap(unsigned** first, unsigned** second) {
	unsigned* temp;
	temp = *first;
	*first = *second;
	*second = temp;
}

/// A function which returns how similar 2 strings are
/// Assumes that both point to 2 valid null terminated array of chars.
/// Returns the similarity between them as a percentage
float similarity(std::string str1_, std::string str2_) {
	for (int i = 0; i < int(str1_.length()); i++)
		str1_[i] = toupper(str1_[i]);

	for (int i = 0; i < int(str2_.length()); i++)
		str2_[i] = toupper(str2_[i]);

	// check if strings have same letters (method A)
	auto temp1 = remove_duplicates(str1_) + remove_duplicates(str2_);
	auto temp2 = remove_duplicates(temp1);

	float similarity = 2 * float(temp1.length() - temp2.length()) / float(temp1.length());

	// check similarity (method B)
	char str1[MAX_STR];
	char str2[MAX_STR];

	strncpy_s(str1, str1_.c_str(), sizeof(str1));
	strncpy_s(str2, str2_.c_str(), sizeof(str2));

	size_t len1 = strlen(str1), len2 = strlen(str2);
	float lenLCS;
	unsigned j, k, * previous, * next;
	if (len1 == 0 || len2 == 0)
		return 0;
	previous = (unsigned*)calloc(len1 + 1, sizeof(unsigned));
	next = (unsigned*)calloc(len1 + 1, sizeof(unsigned));
	for (j = 0; j < len2; ++j) {
		for (k = 1; k <= len1; ++k)
			if (str1[k - 1] == str2[j])
				next[k] = previous[k - 1] + 1;
			else next[k] = previous[k] >= next[k - 1] ? previous[k] : next[k - 1];
		swap(&previous, &next);
	}
	lenLCS = (float)previous[len1];
	free(previous);
	free(next);

	float similarity_2 = lenLCS /= len1;
	return 100 * (2 * similarity / 3 + 1 * similarity_2 / 3);
}

/// password strength (version 5)
/// scale 0-100
/// also checks dictionary file in resource
int password_strength(const std::string& username, const std::string& password, std::vector<std::string>& issues)
{
	if (password.empty())
		return 0;

	double strength = 0;

	// adjust strength based on similarity with username
	float similarity_ = similarity(username, password);

	if (similarity_ > 50) {
		// high similarity, lower strength proportionally
		strength = -int(similarity_ - 50);
		issues.push_back("Username and password too similar");
	}

	int length = (int)password.length();

	// number of items of each type
	int hits_lower = 0;
	int hits_upper = 0;
	int hits_digit = 0;
	int hits_special = 0;

	for (int i = 0; i < length; i++) {
		const char char_ = password[i];
		if (islower(char_)) hits_lower++;
		if (isupper(char_)) hits_upper++;
		if (isdigit(char_)) hits_digit++;
	}

	hits_special = length - (hits_lower + hits_upper + hits_digit);

	if (hits_lower < 3) {
		if (hits_lower == 0)
			issues.push_back("No lowercase characters");
		else
			issues.push_back("Few lowercase characters");
	}

	if (hits_upper < 3) {
		if (hits_upper == 0)
			issues.push_back("No uppercase characters");
		else
			issues.push_back("Few uppercase characters");
	}

	if (hits_special < 3) {
		if (hits_special == 0)
			issues.push_back("No special characters");
		else
			issues.push_back("Few special characters");
	}

	if (hits_digit < 3) {
		if (hits_digit == 0)
			issues.push_back("No digits");
		else
			issues.push_back("Few digits");
	}

	// check for duplicate characters
	int dups_ = number_of_duplicates(password);

	if (dups_ > 2)
		issues.push_back("Duplicate characters");

	// make calculations ////////////////////////////////////////////////////////////////////////

	// add strength at different proportions for different elements
	strength = strength + hits_lower * 3 + hits_upper * 3 + hits_digit * 3 + hits_special * 4;

	// add strength according to complexity of mixing
	double mix_factor =
		(hits_lower + 0) * (hits_upper + 0) +
		(hits_lower + 0) * (hits_digit + 0) +
		(hits_lower + 0) * (hits_special + 0) +
		(hits_upper + 0) * (hits_digit + 0) +
		(hits_upper + 0) * (hits_special + 0) +
		(hits_digit + 0) * (hits_special + 0);

	strength = strength + mix_factor - dups_ * 2.5;

	// impose limits
	if (strength < 0) strength = 0;
	if (strength > 100) strength = 100;

	// check if word is in the dictionary

	// load dictionary into memory
	// NOTE: this function does nothing if the dictionary is already loaded into memory
	dictionary::LoadDict();

	const std::wstring sPassWrd = std::wstring(password.begin(), password.end());
	int iRes = dictionary::SearchInDict(sPassWrd, false);

	if (iRes == 1) {
		strength = strength / 3;
		issues.clear();
		issues.push_back("Password vulnerable to dictionary attack");
	}
	else
		if (iRes == -1) {
			strength = 0;
			issues.clear();
			issues.push_back("Failed to open dictionary resource");
		}

	// end calculations /////////////////////////////////////////////////////////////////////////

	return (int)(strength + 0.5);	// convert to integer
}

liblec::cui::password_quality liblec::cui::password_rating(const std::string& username,
	const std::string& password) {
	password_quality quality;

	quality.strength = password_strength(username, password, quality.issues);

	for (size_t i = 0; i < quality.issues.size(); i++) {
		const auto s = quality.issues[i];

		if (i == 0)
			quality.issues_summary = s;
		else
			quality.issues_summary += ", " + s;
	}

	if (!password.empty()) {
		quality.rating_summary = "Strength = " + std::to_string(quality.strength) + "%";

		if (quality.strength < 20)
			quality.rating_summary += " (Very weak)";
		else {
			if (quality.strength < 50)
				quality.rating_summary += " (Weak)";
			else {
				if (quality.strength < 65)
					quality.rating_summary += " (Average)";
				else {
					if (quality.strength < 80)
						quality.rating_summary += " (Good)";
					else {
						if (quality.strength < 90)
							quality.rating_summary += " (Very good)";
						else
							quality.rating_summary += " (Excellent)";
					}
				}
			}
		}
	}

	return quality;
}
