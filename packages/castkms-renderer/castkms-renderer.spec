Name:           castkms-renderer
Version:        0.1.0
Release:        1%{?dist}
Summary:        Trusted GPU renderer for CastKMS virtual displays

License:        MIT
URL:            https://github.com/pronkproject/castkms-renderer
Source0:        %{name}-%{version}.tar.gz
Source1:        %{name}-%{version}-vendor.tar.xz

BuildRequires:  cargo >= 1.85
BuildRequires:  rust >= 1.83
BuildRequires:  systemd-rpm-macros
Requires:       vulkan-loader

%description
The CastKMS renderer composes virtual display scenes on a GPU and publishes
final images for capture. Its system service holds the administrator capability
needed to issue renderer endpoints. It has no network access or modesetting
authority.


%prep
%autosetup
tar -xJf %{SOURCE1}


%build
export CARGO_NET_OFFLINE=true
cargo build --locked --release -p castkms-rendererd


%install
install -D -m 0755 target/release/castkms-rendererd \
  %{buildroot}%{_libexecdir}/castkms/castkms-rendererd
install -D -m 0644 systemd/castkms-renderer.service \
  %{buildroot}%{_unitdir}/castkms-renderer.service
install -d %{buildroot}%{_unitdir}/multi-user.target.wants
ln -s ../castkms-renderer.service \
  %{buildroot}%{_unitdir}/multi-user.target.wants/castkms-renderer.service


%check
export CARGO_NET_OFFLINE=true
cargo test --locked --workspace


%files
%license LICENSE
%doc README.md
%{_libexecdir}/castkms/castkms-rendererd
%{_unitdir}/castkms-renderer.service
%{_unitdir}/multi-user.target.wants/castkms-renderer.service


%changelog
* Thu Sep 24 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-1
- Package the privileged CastKMS GPU renderer
