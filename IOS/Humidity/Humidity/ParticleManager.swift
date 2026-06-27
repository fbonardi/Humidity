import Combine
import Foundation

@MainActor
final class ParticleManager: ObservableObject {
    @Published var isAuthenticated = false
    @Published var isLoading = false
    @Published var errorMessage: String?

    @Published var deviceName: String?
    @Published var deviceConnected = false
    @Published var soilMoisturePercent: Int?
    @Published var temperatureCelsius: Double?
    @Published var humidityPercent: Double?
    @Published var lastUpdated: Date?

    private var device: ParticleDevice?
    private var refreshTimer: AnyCancellable?
    private let refreshInterval: TimeInterval = 30

    init() {
        isAuthenticated = ParticleCloud.sharedInstance().isAuthenticated
        if isAuthenticated {
            fetchDevice()
        }
    }

    func login(email: String, password: String) {
        isLoading = true
        errorMessage = nil
        ParticleCloud.sharedInstance().login(withUser: email, password: password) { [weak self] error in
            Task { @MainActor in
                guard let self else { return }
                self.isLoading = false
                if let error {
                    self.errorMessage = error.localizedDescription
                    return
                }
                self.isAuthenticated = true
                self.fetchDevice()
            }
        }
    }

    func logOut() {
        ParticleCloud.sharedInstance().logout()
        stopPolling()
        device = nil
        isAuthenticated = false
        soilMoisturePercent = nil
        temperatureCelsius = nil
        humidityPercent = nil
        deviceName = nil
        lastUpdated = nil
    }

    func refreshNow() {
        readAllVariables()
    }

    private func fetchDevice() {
        isLoading = true
        ParticleCloud.sharedInstance().getDevices { [weak self] devices, error in
            Task { @MainActor in
                guard let self else { return }
                self.isLoading = false
                if let error {
                    self.errorMessage = error.localizedDescription
                    return
                }
                guard let device = devices?.first(where: { $0.variables.keys.contains("soilMoisture") }) ?? devices?.first else {
                    self.errorMessage = "No claimed devices found."
                    return
                }
                self.device = device
                self.deviceName = device.name
                self.deviceConnected = device.connected
                self.readAllVariables()
                self.startPolling()
            }
        }
    }

    private func startPolling() {
        refreshTimer = Timer.publish(every: refreshInterval, on: .main, in: .common)
            .autoconnect()
            .sink { [weak self] _ in
                self?.readAllVariables()
            }
    }

    private func stopPolling() {
        refreshTimer?.cancel()
        refreshTimer = nil
    }

    private func readAllVariables() {
        guard let device else { return }

        device.getVariable("soilMoisture") { [weak self] value, error in
            Task { @MainActor in
                guard let self else { return }
                self.deviceConnected = device.connected
                if error == nil {
                    self.soilMoisturePercent = (value as? NSNumber)?.intValue
                    self.lastUpdated = Date()
                }
            }
        }

        device.getVariable("temperature") { [weak self] value, error in
            Task { @MainActor in
                guard let self else { return }
                if error == nil {
                    self.temperatureCelsius = (value as? NSNumber)?.doubleValue
                }
            }
        }

        device.getVariable("humidity") { [weak self] value, error in
            Task { @MainActor in
                guard let self else { return }
                if error == nil {
                    self.humidityPercent = (value as? NSNumber)?.doubleValue
                }
            }
        }

        if errorMessage != nil {
            errorMessage = nil
        }
    }
}
