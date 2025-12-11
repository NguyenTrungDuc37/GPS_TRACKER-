# 1. Bài toán thực tế

## 1.1 Nhu cầu thực tiễn

Sự phát triển mạnh mẽ của các dịch vụ vận chuyển công nghệ như **Grab, Gojek, Be** đã cho thấy nhu cầu cấp thiết về một hệ thống **quản lý đội xe và tài xế từ xa**. Khi số lượng xe và tài xế hoạt động liên tục ngoài đường, việc theo dõi bằng phương pháp thủ công trở nên kém hiệu quả và tồn tại nhiều hạn chế:

- Khó kiểm soát **vị trí xe theo thời gian thực**
- Không theo dõi được **tốc độ và hành vi lái xe**
- Không nắm được **danh tính tài xế** mỗi khi nhận xe
- Không giám sát được **nhiệt độ – độ ẩm cabin**, tiềm ẩn nguy cơ hỏng hóc thiết bị hoặc ảnh hưởng sức khỏe tài xế
- Tốn nhiều **nhân lực giám sát**, dễ sai sót trong quản lý

Những vấn đề trên ảnh hưởng trực tiếp đến:

- **An toàn của tài xế và phương tiện**
- **Chất lượng dịch vụ vận chuyển**
- **Hiệu quả điều phối xe trong giờ cao điểm**
- **Chi phí vận hành của doanh nghiệp**

### 1.2 Ý tưởng và động lực thực hiện
Việc xây dựng một **hệ thống IoT giám sát đội xe** sử dụng **ESP32** kết hợp với các công nghệ:

- **GPS** để lấy vị trí và tốc độ
- **RFID** để nhận dạng tài xế đăng nhập
- **Cảm biến DHT** để theo dõi nhiệt độ – độ ẩm cabin
- **MQTT** để truyền dữ liệu real-time lên server / dashboard

là hoàn toàn cần thiết và mang tính ứng dụng cao.

Hệ thống này giúp các doanh nghiệp vận tải, dịch vụ xe công nghệ hoặc đơn vị logistics có thể xây dựng một nền tảng:

- **Giá rẻ**
- **Linh hoạt**
- **Dễ mở rộng**
- **Tự do tùy biến theo nhu cầu thực tế**
# 2. Nội dung chính
## 2.1 Phần cứng 
- **Vi điều khiển** 
    - ESP32 DEVKIT V1 
- **Định vị và theo dõi**
    - 1 x GPS NEO-6M 
- **Nhân dạng tài xế**
    - 1 x RFID MFRC522 
- **Giám sát môi trường cabin**
    - 1 x DHT11 
- **Cảnh báo**
    - 1 x Buzzer 
- **Đèn trạng thái**
    - 1 x LED RGB
### 2.2 Phần mềm
**Node-RED Dashboard**
- **Hiển thị vị trí xe trên bản đồ (Leaflet/OSM)** 
- **Trạng thái GPS: tốc độ, HDOP, số vệ tinh**
- **Thông tin tài xế đang đăng nhập**
- **Dữ liệu cảm biến cabin**
- **Lịch sử hành trình (GPS history)**
- **Trạng thái kết nối MQTT , WIFI**
**MQTT Broker**
- **EMQX/HiveMQ**
**Phần mềm phát triển**
- **Platform-IO**

# 3. Sơ đồ khối
![Sơ đồ khối chung](images/luudo.png)




