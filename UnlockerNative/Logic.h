#pragma once

class CForceDel;

namespace ForceDel
{
	// Next is the managed wrapper of Logic:
	public ref class Logic
	{
	public:
		// Managed wrappers are generally less concerned 
		// with copy constructors and operators, since .NET will
		// not call them most of the time.
		// The methods that do actually matter are:
		// The constructor, the "destructor" and the finalizer
		Logic();
		~Logic();
		!Logic();

		int Get();
		void Destroy();
		static void InitializeLibrary(System::String^ path);

		void Unlock(System::Collections::Generic::List<System::String^>^ paths);

	private:
		// Pointer to our implementation
		CForceDel* _impl;
	};
}