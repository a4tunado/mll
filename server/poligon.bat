SET PYTHON=C:\Python26\python
SET WSDL2PY=E:\Python26\Scripts\wsdl2py
SET URL=http://poligon.machinelearning.ru/processingservice.asmx?WSDL

REM Uncomment this to build SOAP objects
%PYTHON% %WSDL2PY% -b %URL%

REM %PYTHON% poligon.py

pause