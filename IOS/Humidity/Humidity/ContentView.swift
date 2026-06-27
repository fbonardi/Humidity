import SwiftUI

struct ContentView: View {
    @StateObject private var particle = ParticleManager()

    var body: some View {
        NavigationView {
            Group {
                if particle.isAuthenticated {
                    DeviceStatusView(particle: particle)
                } else {
                    LoginView(particle: particle)
                }
            }
            .navigationTitle("Humidity")
        }
    }
}

private struct LoginView: View {
    @ObservedObject var particle: ParticleManager
    @State private var email = ""
    @State private var password = ""

    var body: some View {
        Form {
            Section("Particle account") {
                TextField("Email", text: $email)
                    .textContentType(.emailAddress)
                    .keyboardType(.emailAddress)
                    .autocorrectionDisabled()
                SecureField("Password", text: $password)
                    .textContentType(.password)
            }

            if let errorMessage = particle.errorMessage {
                Text(errorMessage)
                    .foregroundColor(.red)
            }

            Section {
                Button(particle.isLoading ? "Logging in…" : "Log In") {
                    particle.login(email: email, password: password)
                }
                .disabled(particle.isLoading || email.isEmpty || password.isEmpty)
            }
        }
    }
}

private struct DeviceStatusView: View {
    @ObservedObject var particle: ParticleManager

    private var isDry: Bool {
        (particle.soilMoisturePercent ?? 100) < 30
    }

    var body: some View {
        VStack(spacing: 24) {
            VStack(spacing: 8) {
                Text(particle.deviceName ?? "Device")
                    .font(.headline)
                Label(particle.deviceConnected ? "Connected" : "Offline",
                      systemImage: particle.deviceConnected ? "wifi" : "wifi.slash")
                    .foregroundColor(particle.deviceConnected ? .green : .secondary)
                    .font(.caption)
            }

            if let percent = particle.soilMoisturePercent {
                VStack(spacing: 4) {
                    Text("\(percent)%")
                        .font(.system(size: 64, weight: .bold))
                        .foregroundColor(isDry ? .orange : .blue)
                    Text(isDry ? "Soil is dry — time to water" : "Soil moisture looks good")
                        .foregroundColor(.secondary)
                }
            } else {
                ProgressView("Reading soil moisture…")
            }

            if let lastUpdated = particle.lastUpdated {
                Text("Updated \(lastUpdated.formatted(date: .omitted, time: .standard))")
                    .font(.caption2)
                    .foregroundColor(.secondary)
            }

            if let errorMessage = particle.errorMessage {
                Text(errorMessage)
                    .foregroundColor(.red)
                    .font(.caption)
            }

            Button("Refresh") {
                particle.refreshNow()
            }
            .buttonStyle(.bordered)

            Spacer()

            Button("Log Out", role: .destructive) {
                particle.logOut()
            }
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
