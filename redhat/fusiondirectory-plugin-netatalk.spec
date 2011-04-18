
# Some sort of "detection" of suse
%{?suse_version:%define suse 1}
%{!?suse_version:%define suse 0}

# Define Packagename, e.g.:
# rpmbuild --rebuild --define 'sourcename fusiondirectory' fusiondirectory.srpm
%{!?sourcename:%define sourcename %{name}-%{version}}

#
# Distribution
#
Summary:                Apple account management
Name:                   fusiondirectory-plugin-netatalk
Version:                1.0
Release:                1
License:                GPL
Source:                 ftp://oss.GONICUS.de/pub/fusiondirectory/%{sourcename}.tar.bz2
URL:                    https://oss.gonicus.de/labs/fusiondirectory/
Group:                  System/Administration
Vendor:                 GONICUS GmbH
Packager:               Cajus Pollmeier <pollmeier@gonicus.de>
Buildarch:              noarch
%if %{suse}
Requires:               fusiondirectory >= 1.0, fusiondirectory-plugin-systems
%else
Requires:               fusiondirectory >= 1.0, fusiondirectory-plugin-systems
%endif
BuildRoot:              %{_tmppath}/%{name}-%{version}-root
BuildArch:              noarch

%if %{suse}
        %{echo:Building SuSE rpm}
        %define docdir /usr/share/doc/packages/fusiondirectory
%else
        %{echo:Building other rpm}
        %define webconf /etc/httpd/conf.d/
        %define docdir /usr/share/doc/fusiondirectory-%{version}
%endif

%description
Apple account management

%prep
%setup -q -n %{sourcename}

%build

%install

# Move extra HTML and images
mkdir -p %{buildroot}/usr/share/fusiondirectory/html/plugins/netatalk

echo "%{buildroot} <<---------" 

if ls ./html/* &> /dev/null ;then
	mv ./html/* %{buildroot}/usr/share/fusiondirectory/html/plugins/netatalk
fi

# Move contrib if available
mkdir -p %{buildroot}/usr/share/doc/%{name}
if ls ./contrib/* &> /dev/null ;then
	mv ./contrib %{buildroot}/usr/share/doc/%{name}
fi

# Move help
mkdir -p %{buildroot}/usr/share/fusiondirectory/doc/plugins/netatalk
if ls ./help/* &> /dev/null ;then
	mv ./help/* %{buildroot}/usr/share/fusiondirectory/doc/plugins/netatalk
fi

# Copy etc
mkdir -p %{buildroot}/etc/fusiondirectory
if ls ./etc/* &> /dev/null ;then
	mv ./etc/* %{buildroot}/etc/fusiondirectory
fi

# Move localization
mkdir -p %{buildroot}/usr/share/fusiondirectory/locale/plugins/netatalk
if ls ./locale/* &> /dev/null ;then
	mv ./locale/* %{buildroot}/usr/share/fusiondirectory/locale/plugins/netatalk
fi

# Move plugin itself
mkdir -p %{buildroot}/usr/share/fusiondirectory/plugins/
mv ./* %{buildroot}/usr/share/fusiondirectory/plugins/

%post
/usr/sbin/update-fusiondirectory

%postun
/usr/sbin/update-fusiondirectory

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%attr (-,root,root) /usr/share/fusiondirectory/plugins/
%attr (-,root,root) /usr/share/fusiondirectory/doc/plugins/netatalk
%attr (-,root,root) /usr/share/fusiondirectory/locale/plugins/netatalk
%attr (-,root,root) /usr/share/fusiondirectory/html/plugins/netatalk
%attr (-,root,root) /usr/share/doc/%{name}
%attr (-,root,root) /etc/fusiondirectory/