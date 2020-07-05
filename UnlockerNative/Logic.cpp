#include "stdafx.h"
#include "Logic.h"
#include "ForceDel.h"
#include <string>

using namespace std;
using namespace ForceDel;

Logic::Logic()
	: _impl(new CForceDel()) // Allocate some memory for the native implementation	
{
}

int Logic::Get()
{
	return 0;// _impl->Get(); // Call native Get
}

void Logic::Destroy()
{
	if (_impl != nullptr)
	{
		delete _impl;
		_impl = nullptr;
	}
}

Logic::~Logic()
{
	// C++ CLI compiler will automaticly make all ref classes implement IDisposable.
	// The default implementation will invoke this method + call GC.SuspendFinalize.
	Destroy(); // Clean-up any native resources 
}

Logic::!Logic()
{
	// This is the finalizer
	// It's essentially a fail-safe, and will get called
	// in case Logic was not used inside a using block.
	Destroy(); // Clean-up any native resources 
}

string ManagedStringToStdString(System::String^ str)
{
	cli::array<unsigned char>^ bytes = System::Text::Encoding::ASCII->GetBytes(str);
	pin_ptr<unsigned char> pinned = &bytes[0];
	std::string nativeString((char*)pinned, bytes->Length);
	return nativeString;
}

void Logic::InitializeLibrary(System::String^ path)
{
	string nativePath = ManagedStringToStdString(path);
	LoadLibrary(nativePath.c_str()); // Actually load the delayed library from specific location
}


void Logic::Unlock(System::Collections::Generic::List<System::String^>^ paths)
{
	for each (System::String^ path in paths)
	{
		string nativePath = ManagedStringToStdString(path);
		int result = _impl->Run(nativePath.c_str(), FALSE, FALSE);
	}
}
