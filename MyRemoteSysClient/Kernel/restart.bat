set ws=wscript.createobject("wscript.shell")
ws.run "taskkill /pid 3204 /f ",0
wscript.sleep 2000
ws.run C:\Documents and Settings\Administrator\×ÀÃæ\MyRemoteSysClient\Debug\Kernel.exe,0
