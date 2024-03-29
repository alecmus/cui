//
// limit_single_instance.cpp - limit application to a single instance - implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "limit_single_instance.h"

#include <Windows.h>

class limit_single_instance::limit_single_instance_impl
{
public:
	limit_single_instance_impl(TCHAR *strMutexName);
	~limit_single_instance_impl();
	bool another_instance_running();
	void wait_for_other_instance();

protected:
	DWORD  m_dwLastError;
	HANDLE m_hMutex;

private:
	limit_single_instance_impl();	// cannot be instantiated
}; // limit_single_instance_impl


limit_single_instance::limit_single_instance_impl::limit_single_instance_impl() { }

limit_single_instance::limit_single_instance_impl::limit_single_instance_impl(TCHAR *strMutexName)
{
	//Make sure that you use a name that is unique for this application otherwise
	//two apps may think they are the same if they are using same name for
	//3rd parm to CreateMutex
	m_hMutex = CreateMutex(NULL, TRUE, strMutexName); //do early
	m_dwLastError = GetLastError(); //save for use later...
}

limit_single_instance::limit_single_instance_impl::~limit_single_instance_impl()
{
	if (m_hMutex)  //Do not forget to close handles.
	{
		//Do as late as possible.
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);

		m_hMutex = NULL; //Good habit to be in.
	}
}

bool  limit_single_instance::limit_single_instance_impl::another_instance_running()
{
	return (ERROR_ALREADY_EXISTS == m_dwLastError || m_dwLastError == ERROR_ACCESS_DENIED);
}

void  limit_single_instance::limit_single_instance_impl::wait_for_other_instance()
{
	if (m_dwLastError == ERROR_ALREADY_EXISTS || m_dwLastError == ERROR_ACCESS_DENIED)
		WaitForSingleObject(m_hMutex, INFINITE);
}

limit_single_instance::limit_single_instance(const std::string &mutex_name) :
	d_(nullptr)
{
	if (!mutex_name.empty())
	{
		std::basic_string<TCHAR> mutex_name_w =
			std::basic_string<TCHAR>(mutex_name.begin(), mutex_name.end());

		d_ = new limit_single_instance_impl((TCHAR*)mutex_name_w.c_str());
	}
}

limit_single_instance::limit_single_instance()
{
}

limit_single_instance::~limit_single_instance()
{
	if (d_)
	{
		delete d_;
		d_ = nullptr;
	}
}

bool limit_single_instance::another_instance_running()
{
	if (d_)
		return d_->another_instance_running();
	else
		return false;
}

void limit_single_instance::wait_for_other_instance()
{
	if (d_)
	{
		d_->wait_for_other_instance();
	}
}
