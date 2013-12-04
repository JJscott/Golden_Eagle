@echo off
echo Making symlink to resource directory...
IF EXIST %1 (
	echo Target exists, nothing to do.
) ELSE (
	cmd /S /C " "%~dp0\admincmd.lnk" /S /C mklink /D %1 %2 "
	echo Symlink created.
)
