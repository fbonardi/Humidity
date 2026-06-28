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
    @Published var soilTempCelsius: Double?
    @Published var temperatureCelsius: Double?
    @Published var humidityPercent: Double?
    @Published var pressureHPa: Double?
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
        soilTempCelsius = nil
        temperatureCelsius = nil
        humidityPercent = nil
        pressureHPa = nil
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

        // fixes #1/#2/#3/#7/#8: surface errors, guard against post-logOut writes,
        // update deviceConnected and lastUpdated in every callback
        device.getVariable("soilMoisture") { [weak self] value, error in
            Task { @MainActor in
                guard let self, self.isAuthenticated else { return }
                self.deviceConnected = device.connected
                if let error {
                    self.errorMessage = error.localizedDescription
                } else {
                    self.soilMoisturePercent = (value as? NSNumber)?.intValue
                    self.lastUpdated = Date()
                    self.errorMessage = nil
                }
            }
        }

        device.getVariable("soilTemp") { [weak self] value, error in
            Task { @MainActor in
                guard let self, self.isAuthenticated else { return }
                self.deviceConnected = device.connected
                if error == nil {
                    self.soilTempCelsius = (value as? NSNumber)?.doubleValue
                    self.lastUpdated = Date()
                }
            }
        }

        device.getVariable("temperature") { [weak self] value, error in
            Task { @MainActor in
                guard let self, self.isAuthenticated else { return }
                self.deviceConnected = device.connected
                if error == nil {
                    self.temperatureCelsius = (value as? NSNumber)?.doubleValue
                    self.lastUpdated = Date()
                }
            }
        }

        device.getVariable("humidity") { [weak self] value, error in
            Task { @MainActor in
                guard let self, self.isAuthenticated else { return }
                self.deviceConnected = device.connected
                if error == nil {
                    self.humidityPercent = (value as? NSNumber)?.doubleValue
                    self.lastUpdated = Date()
                }
            }
        }

        device.getVariable("pressure") { [weak self] value, error in
            Task { @MainActor in
                guard let self, self.isAuthenticated else { return }
                if error == nil {
                    self.pressureHPa = (value as? NSNumber)?.doubleValue
                }
            }
        }
    }
}
