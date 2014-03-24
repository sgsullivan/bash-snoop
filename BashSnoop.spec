Summary: Retrieve the in memory history of a bash process, and/or save its memory heap.
Name: bash-snoop
Version: 0.0.1
Release: 0
Group: System Tools/Utilities
URL: http://ssullivan.org
License: GPL
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildArch: x86_64
BuildRequires: glibc-devel
Requires: gdb 

%description
If you've ever wanted to see the command history for a currently running bash
process, BashSnoop can assist with this. This tool has the ability to write a
bash processes command history that currently resides only memory to a file. 
This is useful for "catching" the bash history of potentially malicious SSH
sessions, where the logs otherwise would not have been recorded by the user
clearing the sessions bash history before their final session exit.

%prep
%setup -q -T -D -n bash-snoop

%build
gcc bash-snoop.c -o bash-snoop

%install
mkdir -p ${RPM_BUILD_ROOT}/usr/bin
install -m700 bash-snoop ${RPM_BUILD_ROOT}/usr/bin/bash-snoop

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
/usr/bin/bash-snoop

%post
%postun

%changelog
* Thu Oct 11 2012 Scott Sullivan <scottgregorysullivan@gmail.com> 0.0.1-0
- Initial release.
