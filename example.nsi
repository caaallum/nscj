RequestExecutionLevel user

;Var City
;Var Name

Var Name
Var Result

Section

nscj::Set /url "https://jsonplaceholder.typicode.com/users/1"
Pop $Result
DetailPrint "Fetch: $Result"

nscj::Get "name"
Pop $Name
DetailPrint "Name: $Name"


SectionEnd