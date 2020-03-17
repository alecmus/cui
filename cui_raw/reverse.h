/*
** reverse.h - template for iterating through a vector in reverse
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*/

#pragma once

template<typename It>
class Range
{
	It b, e;
public:
	Range(It b, It e) : b(b), e(e) {}
	It begin() const { return b; }
	It end() const { return e; }
};

template<typename ORange, typename OIt = decltype(std::begin(std::declval<ORange>())), typename It = std::reverse_iterator<OIt>>
Range<It> reverse(ORange && originalRange) {
	return Range<It>(It(std::end(originalRange)), It(std::begin(originalRange)));
}
