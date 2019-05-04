@echo off
for /d %%i in (*) do (
for %%a in (%%i\*.*) do echo %%a
) > %%i\%%i.pri
)

for /d %%i in (*) do (
echo include^(%%i^\%%i.pri^)
) >> Master_190504.pro