
#pragma comment(lib,"bufferoverflowU.lib") 

//emmmm ,link failed with not found _ftol2_sse....
//there is no _ftol2_sse in msvcrt.lib
//but I found the implement of it in Google and it works well......

extern "C" void __declspec(naked) _ftol2_sse()
{
	__asm{
			push        ebp  
			mov         ebp,esp 
			sub         esp,8 
			and         esp,0FFFFFFF8h 
			fstp  qword ptr [esp] 
			cvttsd2si   eax,qword ptr [esp] 
			leave            
			ret
	}
}