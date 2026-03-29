RequestExecutionLevel user

Section

nscj::Set /url "https://jsonplaceholder.typicode.com/users/1"
Pop $0
DetailPrint "Fetch: $0"

nscj::Set /tree user1 /url "https://jsonplaceholder.typicode.com/users/2"
Pop $0
DetailPrint "Fetch: $0"

nscj::Set /tree user2 /url "https://jsonplaceholder.typicode.com/users/3"
Pop $0
DetailPrint "Fetch: $0"

nscj::Get /tree user1 "name"
Pop $0
DetailPrint "Name: $0"

nscj::Get /tree user2 "name"
Pop $0
DetailPrint "Name: $0"

nscj::Get "name"
Pop $0
DetailPrint "Name: $0"

SectionEnd