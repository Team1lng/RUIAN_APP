#include "language.h"
#include "../api/xls/lang_xls.h"
#include "user_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_DEVICE_NUM 6
#define MAX_DEVICE_NAME_LEN 10

/*
注意！！！大部分页面的语言文字可以在本文件修改，
layout_define.c的文字需要去该文件手动修改，相关代码在670行左右，其次，需要用到数组的矩阵也需要手写字符串。
layout_cctv.c   146行左右

*/


static  char *lang_str[LANG_STR_ID_TOTAL][language_total] =
	{
		//layout home
		{"Monitor","监控","Панели","Moniteur","Monitor","Monitor","شاشة","Monitor", "צפייה","Monitor","Monitor","Podgląd","Οθόνη","Monitör","Camera"},
        {"Intercom","内线通话","Интерком","Interphone","Interfono","Sprechanlage","الاتصال الداخلي","Interkom", "חיוג למסך נוסף","Intercomunicador","Interfono","Interkom","Ενδοεπικοινωνία","İçindeki iletişim","Intercom"},
        {"Playback","浏览","Записи","Playback","Reproducción","Wiedergabe","عرض التسجيلات","Záznamy", "הקלטות","Reprodução","Riproduzione","Pamięć","Αναπαραγωγή","Çal","Geheugen"},
        {"Dormancy","休眠模式","Сон","Dormance","Inactividad","Schlafzustand","وضع السكون","Nerušit", "בית שקט","Não Incomodar","Inattività","Tryb nocny","Αδράνεια","Dormancyfrance. Kgm","Nachtmodus"},
        {"At Home","居家模式","Дома","À la maison","En casa","Zuhause","فى المنزل","Doma", "בבית","Em Casa","In Casa","W domu","Στο Σπίτι","Evde","Thuis"},
        {"Leave Home","离家模式","Не дома","Quitter la maison","Salir de casa","Zuhause verlassen","خارج المنزل","Mimo domov", "לא בבית","Ausente","Fuori Casa","Poza domem","Εκτός Σπιτιού","Eve git","Afwezig"},
        {"Door", "大门", "Открыть", "Porte", "Puerta", "Tür","باب","Dveře","שער","Porta","Porta","Wejście","Πόρτα","Kapı","Deur"},
        {"Cancel","取消","Отмена","Annuler","Cancelar","Absagen","الغاء","Zrušit","בוטל","Cancelar","Annulla","Anuluj","Ακύρωση","Lütfen","Annuleer"},
        //layout intercom
        {"Device is busy! Please wait!!!",
        "设备正忙! 请稍等!!!",
        "Устройство занято! пожалуйста ждите!!!",
        "Le périphérique est occupé! S'il vous plaît, attendez!!!",
        "El dispositivo está ocupado! Espere por favor!!!",
        "Gerät ist beschäftigt! Warten Sie mal!!!",
        "!!!الجهاز مشغول !يرجى الانتظار",
        "Zařízení je obsazené!  Prosím, čekejte!!!",
        "ההתקן תפוס נא להמתין!!!",
        "O Dispositivo Está Ocupado! Por Favor, Aguarde",
        "Dispositivo occupato! attendere",
        "Urządzenie zajęte. Spróbój później."
        "Η συσκευή είναι απασχολημένη! Παρακαλώ περιμένετε!!!",
        "Aygıt meşgul! Lütfen bekle!",
        "Apparaat is bezet! Even geduld aub!!!"
    },
        //layout monitor
        {"Photo", "拍照", "Фото", "Photo", "Foto", "Foto", "صور ", "Fotografie", "תמונה","Fotos","Foto","Zdjęcia","Φωτογραφία","Fotoğraf","Foto"},
        {"Record", "录像", "Видео", "Record", "Registro", "Datensatz", "تسجيل ", "Záznam obrazu", "הקלטת וידאו","Vídeos","Registrazione","Film","Καταγραφή","fotoğraf kayıtları","Video"},
        {"Talk", "通话", "Ответить", "Parler", "Hablar", "Reden", "تكلم ", "Hovořit", "דיבור","Falar","Parla","Rozmowa","Μιλήστε","Konuş","Spreek"},
        {"Garage", "车库门", "Открыть", "Garage", "Garaje", "Garage","كراج","Garáž","פתיחת דלת 2","Garagem","Box auto","Brama","Γκαράζ","Garage","Poort"},
        {"Exit", "退出", "Выход", "Sortie", "Salida", "Ausgang", "خروج", "Ukončit", "יציאה","Sair","Uscita","Anuluj","Έξοδος","Çıkın","Terugkeren"},
        //layout photo
        {"\n\nDelete ?\n\n","\n\n删除 ?\n\n","\n\nУдалить ?\n\n","\n\nSupprimez ?\n\n","\n\nEliminar ?\n\n","\n\nLöschen ?\n\n","\n\n? حذف\n\n","\n\nVymazat ?\n\n", "\n\nמחיקה ?\n\n","\n\nApagar\n\n","\n\nElimina ?\n\n","\n\nUsunąć ?\n\n","\n\nΔιαγραφή ?\n\n","\n\nSil ?\n\n","\n\nVerwijderen ?\n\n"},
        //layout playback 
         {"\n\nDelete Selected ?\n\n","\n\n删除已选中的 ?\n\n","\n\nУдалить выбранное ?\n\n","\n\nSupprimer ce qui est sélectionné ?\n\n","\n\n¿Eliminar los seleccionados ?\n\n","\n\nAusgewählte löschen ?\n\n","\n\nحذف مختارة ؟\n\n","\n\nSmazat vybrané ?\n\n", "\n\nמחק את הנבחר ?\n\n","\n\nApagar os Seleccionados?\n\n","\n\nElimina l' elemento selezionato?\n\n","\n\nUsunąć wybrane ?\n\n","\n\nΔιαγραφή επιλεγμένων ?\n\n","\n\nSeçilenleri sil ?\n\n","\n\nGeselecteerde verwijderen ?\n\n"},
        {"\n\nDelete All ?\n\n","\n\n全部删除 ?\n\n","\n\nУдалить все ?\n\n","\n\nSupprimer tout ?\n\n","\n\nBorrar todo ?\n\n","\n\nAlle löschen ?\n\n","\n\n? حذف الكل\n\n","\n\nSmazat všechny ?\n\n", "\n\nמחיקת הכל?\n\n","\n\nApagar Todas\n\n","\n\nElimina tutto\n\n","\n\nUsunąć wszystko ?\n\n","\n\nΔιαγραφή όλων ?\n\n","\n\nHepsini sil ?\n\n","\n\nAlles verwijderen ?\n\n"},
        //layout product_introduction
        {"Products", "产品介绍", "Презентация продукции", "Présentation du produit", "Introducción del producto", "Produkteinführung","عرض المنتجات", "Úvod produktu", "הצגת מוצר","Introdução do produto","Prodotto","Introduzione del prodotto","Produkt","Ürün Tanıtımı","Productintroductie"},
        //layout ring
        {"Set ring", "铃声设置", "Звук", "La cloche", "Tintineo", "Ring", "ضبط الرنين", "Nastavit zvonění", "תכנות צלצול","Definir Toque","Imposta Suoneria","Ustaw dzwonek","Ρύθμιση κουδουνιού","Zil Ayarı","Beltoon instellen"},
        {"Custom ringtones","自定义铃声","Настройка вызова","Sonnerie personnalisée","Timbre personalizado","Klingeltöne anpassen","نغمات مخصصة","Upravit vyzváněcí tóny","תואם צלצולים","Toques Personalizados","Suonerie personalizzate","Dostosowanie dźwięku","Προσαρμογή ήχου κουδουνιού","Özel zil sesleri","Aangepaste beltonen"},
        {"Local ringtone","本地铃声","Локальные мелодии","Une version de logicielSonnerie locale","Timbre local","Lokaler Klingelton","النغمات المحلية","Místní vyzvánění", "צלצול מקומי","Toque Local","Suoneria locale"},
        {"System ring","系统铃声","Системные мелодии","Sonnerie du système","Timbre del sistema","Systemglocke","نظام الرنين","Systémové vyzvánění", "פעמון מערכת","Toque do Sistema","suoneria di sistema"},
        //layout set language
        {"Language","语言","Язык","Langues","Idioma","Sprache","اللغة","Jazyk", "שפה","Idioma","Lingua","Język","Γλώσσα","Dil","Taal"},
        //layout add network
        {"Add network","添加网络","Добавить сеть","Ajouter un réseau" ,"Añadir red","Netzwerk hinzufügen","اضافة شبكة ","Přidat síťové připojení","הוספת רשת","Adicionar Rede Wi-Fi","Aggiungi rete","Dodaj sieć","Προσθήκη δικτύου","Ağ ekle","Netwerk toevoegen"},
        {"Wifi info","Wifi信息","Информация Wi-Fi","Infos Wifi","Información wifi","WLAN-Info","معلومات شبكة الواي فاي","Informace o wifi", "מידע WIFI","Informações de Sistema","Informazioni Wi-Fi","Informacje o Wi-Fi","Πληροφορίες Wi-Fi","WiFi bilgisi","WiFi info"},
        {"IP address:","IP地址:","IP адрес:","Adresse IP:","Dirección IP:","IP-Adresse:","عنوان IP:",	 "IP adresa:", "כתובת IP:","Endereço IP:","Indirizzo IP:","Adres IP:","Διεύθυνση IP:","IP adresi:","IP adres:"},
		{"Security:","安全性:","Безопасность:","  Sécurité:","Seguridad:","Sicherheit:","الامان:", 	 "Zapezpečení:", "אבטחה:","Segurança:","Sicurezza","Bezpieczeństwo:","Ασφάλεια:","Güvenlik:","Beveiliging:"},
        //layout setting common
        {"Network device resetting","网络设备重置中","Сброс сетевого устройства","Réinitialisation du périphérique réseau","Se está restableciendo el dispositivo de red","Netzwerkgerät zurücksetzen","إعادة تعيين جهاز الشبكة","Resetování síťového zařízení","מתקן רשת מתקן מחדש","Redefinição do dispositivo de rede","Ripristino del dispositivo di rete","Resetowanie urządzenia sieciowego","Επαναφορά συσκευής δικτύου","Ağ cihazı sıfırlanıyor","Netwerkapparaat wordt gereset"},
        //layout setting password
        {"Please input a password","请输入密码","Пожалуйста введите пароль","Veuillez saisir le mot de passe","Introduzca la contraseña","Bitte geben Sie ein Passwort ein","الرجاء إدخال كلمة السر","Zadejte prosím heslo","אנא הכנס סיסמה","Introduza por favor uma senha","Inserisci password","Wprowadź hasło","Παρακαλώ εισάγετε έναν κωδικό πρόσβασης","Lütfen bir şifre girin","Voer een wachtwoord in"},
        {"Incorrect password","密码不正确","Некорректный пароль","Incorrect password","Contraseña incorrecta","Falsches Passwort","كلمة السر غير صحيحة","Nesprávné heslo","סיסמה לא נכונה","Senha incorrecta","Password errata","Nieprawidłowe hasło","Λάθος κωδικός πρόσβασης","Yanlış şifre","Onjuist wachtwoord"},
        {"Password reset", "密码重置","Сброс пароля", "Réinitialisation du mot de passe",  "restablecer la contraseña", "Passwort zurücksetzen","إعادة تعيين كلمة المرور","Obnovení hesla","מחדש את הסיסמה","Repor a Senha","Reset Password","Resetowanie hasła","Επαναφορά κωδικού πρόσβασης","Şifre sıfırlama","Wachtwoord resetten"},
        {"New password", "新密码", "Новый пароль", "Nouveau mot de passe","Nueva Schwester", "Neues Passwort", "كلمة المرور الجديدة","Nové heslo","סיסמה חדשה","Nova senha","Nuova password","Nowe hasło","Νέος κωδικός πρόσβασης","Yeni şifre","Nieuw wachtwoord"},
        {"Confirm Password", "确认密码", "Подтвердить пароль", "Confirmer le mot de passe","Confirmar contraseña", "Bestätigen Sie Ihr Passwort","تأكيد كلمة المرور","Potvrzení hesla","אישור את הסיסמה","Confirmar a Senha","Conferma password","Potwierdź hasło","Επιβεβαίωση κωδικού πρόσβασης","Şifreyi Onayla","Bevestig wachtwoord"},
        {"Passwords do not match","密码不匹配", "Неверный пароль", "Le mot de passe ne correspond pas","La contraseña no coincide", "Passwörter stimmen nicht überein", "كلمة المرور غير متطابقة", "Hesla se neshodují","הסיסמאות לא מתאימות","As senhas não correspondem","le password non corrispondono","Hasła nie pasują do siebie","Οι κωδικοί πρόσβασης δεν ταιριάζουν","Parolalar eşleşmiyor","Wachtwoorden komen niet overeen"},
        {"apply",  "应用", "Применить", "appliquer","Aplicar", "verwenden", "تأكيد","použite","אפליקציה","aplicar","Applica","zastosuj","εφαρμογή","uygulama","toepassen"},
        //layout setting room no
        {"Your current room number","你当前的房号","Ваш текущий номер","Votre numéro de chambre actuel","Su número de habitación actual","Ihre aktuelle Zimmernummer","رقم غرفتك الحالية","Vaše aktuální číslo pokoje","מספר החדר הנוכחי שלך","O seu número actual do quarto","Il tuo attuale numero di camera","Twój aktualny numer pokoju","Ο τρέχων αριθμός δωματίου σας","Mevcut oda numaranız","Uw huidige kamernummer"},
        {"Please enter the room number to be set\nPress # to confirm","请输入要设置的房号\n按#确认","Введите номер комнаты\nНажмите кнопку #, чтобы подтвердит","Veuillez saisir le numéro de chambre à définir\nAppuyez sur le  # pour confirmer",\
        "Introduzca el número de habitación que desea establecer\nConfirme con #","Bitte geben Sie die Zimmernummer ein,\ndie Sie einstellen möchten.\nDrücken Sie # zur Bestätigung.",\
        "أدخل رقم الغرفة التي تريد تعيين\nاضغط على # للتأكيد","Zadejte prosím číslo pokoje\nkteré má být nastaveno\nStiskněte # pro potvrzení","אנא הכנס את מספר החדר שיקבע\nלחץ # כדי לאשר","Introduza o número do quarto a definir\nPressione # para confirmar","Inserire numero camera da impostare \n premere # per confermare"},
        //layout setting 
        {"Set room ID", "房号设置", "ID домофона", "Chambre Non", "Habitación no", "Raum Nr", "ضبط معرف الغرفة", "Nastavte ID místnosti", "תכנות מספר מסך", "Definir ID da Zona", "Imposta l'ID stanza"," Ustaw ID pokoju","Ρύθμιση αναγνωριστικού δωματίου","Oda Kimliği Ayarla","Kamer-ID instellen"},
        {"Set time", "时间设置", "Время", "Temps", "Tiempo", "Zeit", "ضبط الوقت", "Nastavte čas", "תכנות שעון", "Definir Hora", "Tempo impostato","Ustaw czas","Ρύθμιση ώρας","Zaman Ayarla","Tijd instellen"},
        {"Screen saver", "屏保设置", "Экранная заставка", "Écran de veille", "Salvapantallas", "Screensaver", "شاشة التوقف", "Spořič obrazovky", "שומר מסך", "Protector de Tela", "Salvaschermo","Wygaszacz ekranu","Προφύλαξη οθόνης","Ekran koruyucu","Schermbeveiliging"},
        {"Other", "其它设置", "Другой", "Autres", "Otros", "Andere", "اخرى", "Jiné", "אחר", "Outros", "Altro","Inne","Άλλο","Başka","Andere"},
        {"Reset", "重置", "Перезагрузить", "Réinitialiser", "Reiniciar", "Zurücksetzen", "اعادة الضبط", "Resetovat", "ביצוע ריסט", "Redefinir", "Ripristina"},
        {"Ring", "铃声", "Звонок", "Anneau", "Anillo", "Ring", "رنين", "Zvonění", "رنين ", "Toque", "Suoneria"," Dzwonek","Κουδούνι","Zil","Bel"},
        {"Ring time", "铃声时间", "Время звонка", "Temps de sonnerie", "Tiempo de timbre", "Klingelzeit", "مدة نغمة الرنين", "Doba zvonění", "זמן צלצול", "Tempo de Toque", "Tempo di squillo","Czas trwania dźwięku","Ώρα κουδουνίσματος","Yüzük zamanı","Beltoon tijd"},
        {"Doorbell","门铃音量","Звонок","Sonnette","Timbre","Türklingel","جرس الباب","Domovní \nzvonek","פעמון","Campainha","Campanello","Kamera","Κουδούνι","Kapı zili","Deurbel"},
        {"Ringtone","内呼音量","Рингтон","Ringtone","Tintineo","Klingelton","نغمة الرنين","Vyzváněcí tón","סוג צלצול","Tom de Toque","Suoneria","Interkom","Ήχος","Ringtone","Beltoon"},
        {"Device ID", "设备ID", "ID устройства", "ID du périphérique", "ID del dispositivo", "Geräte-ID", "معرف الجهاز", "ID zařízení", "זהות המכשיר", "ID do dispositivo", "Ripetere ID","Adres monitora"," ID Συσκευής","Aygıt ID","Apparaat ID"},
        {"Confirm", "确认", "Подтвердить", "Confirmer", "Confirmar", "Bestätigen Sie", "تاكيد", "Potvrdit", "אישור", "Confirmar", "Conferma","Potwierdź","Επιβεβαίωση","Onayla","Bevestigen"},
        {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול", "Cancelar", "Annulla","Anuluj","Ακύρωση","Lütfen","Annuleer"},
        {"Formatting", "格式化", "Форматирование", "Formatage", "Formateo", "Formatierung", "جار التهيئة", "Formátování", "מאתחל", "A Formatar", "Formattazione","Formatowanie","Μορφοποίηση","Formatlama","Formateren"},
        {"Video", "录像", "Видео", "Vidéo", "Video", "Video", "فديو ", "Video", "וידאו", "Vídeos", "Registrazione","Wideo","Βίντεο","Video","Video"},
        {"OFF", "关", "Выключено", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור", "Desligado", "Desligado","Wył.","OFF","Kapalı","Uit"},
        {"ON", "开", "Включено", "Ouvert", "Abierto", "Öffnen", "تشغيل", "Zap.", "פועל", "Ligado", "Ligado","Wł.","ON","Aç","Aan"},
        {"Record", "自动拍照", "Запись", "Record", "Registro", "Datensatz", "تسجيل", "Záznam", "הקלטה", "Gravar", "Registrazione","Zapis do pamięci","Καταγραφή","Kaydet","Video opnemen"},
        {"Motion Detection", "移动侦测", "Обнаружение движения", "Détection de mouvement", "Detección de movimiento", "Bewegungserkennung", "كشف الحركة", "Detekce pohybu", "זיהוי תנועה", "Detecção de Movimento", "Detecção de Movimento","Detekcja ruchu","Ανίχνευση Κίνησης","Hareket Keşfetmesi","Bewegingsdetectie"},
        {"Door unlock delay", "大门开锁延时", "Задержка разблокировки двери", "Délai de déverrouillage de la porte", "Retardo de desbloqueo de puerta", "Tür Entriegelungsverzögerung", "تأخير فتح الباب ", "Zpoždění odemknutí dveří", "השהיית פתיחת דלת", "Atraso na Abertura da Porta", "Atraso na Abertura da Porta","Opóźnienie odblokowania drzwi","Καθυστέρηση ξεκλειδώματος πόρτας","Kapı açma gecikmesi","Deur ontgrendelingsvertraging"},
        {"Garage unlock delay", "车库门开锁延时", "Задержка разблокировки ворот", "Délai de déverrouillage de la garage", "Retardo de desbloqueo de garaje", "Garage Entriegelungsverzögerung", "تأخير فتح باب المرآب", "Zpoždění odemknutí garáže", "השהיית פתיחת דלת 2", "Atraso na Abertura da Garagem", "Atraso na Abertura da Garagem","Opóźnienie odblokowania bramy","Καθυστέρηση ξεκλειδώματος γκαράζ","Garaj açma gecikmesi","Garage ontgrendelingsvertraging"},
        {
            "Please wiat!!! Upgrading outdoor!!!!",
            "请稍等!!! 门口机升级中!!!",
            "Пожалуйста ждите!!! Обновление наружного устройства!!!",
            "Veuillez patienter!!! Mise à niveau en plein air!!!",
            "Espere por favor!!! Actualización al aire libre!!!",
            "Bitte warten!!! Outdoor aufrüsten!!!",
            "يرجى الانتظار جار الترقية!!!",
            "Prosím počkej!!! Aktualizuji zařízení!!!",
            "נא להמתין משדרג יחידה חיצונית",
            "A Actualizar Unidade Exterior!!! Por favor, Aguarde",
            "Attendere!!! Aggiornamento esterno",
            "Proszę czekać!!! Aktualizacja urządzenia zewnętrznego!!!",
            "Παρακαλώ περιμένετε!!! Αναβάθμιση εξωτερικήςς συσκευής!!!",
            "Lütfen bekleyin!!! Dış mekan yükseltiliyor!!!",
            "Even geduld aub!!! Buitenapparaat wordt geüpgraded!!!"
        },
        {
            "Outdoor unit offline!",
            "门口机不在线!",
            "Наружное устройство отключено!",
            "Unité extérieure hors ligne!",
            "Unidad exterior fuera de línea!",
            "Außengerät offline!",
            "!الوحدة الخارجية غير متصلة",
            "Venkovní jednotka offline!",
            "יחידה חיצונית מנותקת",
            "Unidade Exterior Desligada",
            "Unità esterna offline",
            "Jednostka zewnętrzna offline!",
            "Η εξωτερική μονάδα εκτός σύνδεσης!",
            "Dış ünite çevrimdışı!",
            "Buitenunit offline!"
        },
        {"Upgrade", "升级", "Обновлять", "Mise à niveau", "Mejora", "Aufrüstung", "ترقية", "Aktualizace", "שדרוג", "Actualizar", "Aggiornamento","Aktualizacja","Αναβάθμιση","Güncelle","Update"},
        {"Language", "语言", "Язык", "Langues", "Idioma", "Sprache", "اللغة", "Jazyk", "שפה", "Idioma", "Lingua","Język","Γλώσσα","dil","Taal"},
        {"System", "系统", "Система", "Système", "Sistema", "System", "النظام", "Systém", "מערכת", "Sistema", "Sistema","System","Σύστημα","Sistem","Systeem"},
        {"Security password", "安全密码", "Пароль безопасности", "Mot de passe sécurisé", "Contraseña de Seguridad", "Sicherheitspasswort", "كلمة سر آمنة", "Bezpečnostní heslo", "סיסמא ביטחון", "Senha de segurança", "Password di sicurezza","Hasło","Κωδικός ασφαλείας","Güvenlik paroli","Beveiligingswachtwoord"},
        {
            "Please wait a moment,looking for the unit doorway machine",
            "请稍等，正在寻找单元门口机",
            "Пожалуйста подождите, идет поиск устройств",
            "Attendez, je cherche un portier",
            "Por favor, espere un momento, buscando la puerta de la unidad",
            "Bitte warten Sie einen Moment \n Wir suchen das Gerät Türöffnungsmaschine",
            "لحظة من فضلك ، أنا أبحث عن مدخل الوحدة ",
            "Počkejte chvíli. Hledáme stroj pro dveře jednotky",
            "אנא חכה רגע. אנחנו מחפשים את מכונת הדלת היחידה",
            "Por favor, Aguarde um Momento. A procurar Unidade Exterior",
            "Per favore aspetta un momento, sto cercando la porta dell'unità",
            "Proszę czekać, szukam domofonu",
            "Παρακαλώ περιμένετε, ψάχνω την πόρτα της μονάδας",
            "Lütfen bir dakika bekleyin, birim kapı makinesi aranıyor",
            "Even geduld aub, op zoek naar de deur van de eenheid"
        },
        {
            "The unit doorway device has been found \n is waiting for device confirmation\n\
            Please press and hold the call button of the device for more than 5s \n\
            to pair the device",

            "已找到单元门口机设备,正在等待设备确认\n\
            请长按设备的呼叫按键5秒以上进行设备配对",

            "Найдена вызывная панель\n\
            Для ее добавления  \n\
            нажмите и удерживайте кнопку вызова на вызывной панели не менее 5 секунд",

            "Un périphérique de porte d'Unit é a été trouvé et attend la confirmation du périphérique  \n \
            please appuyez longtemps sur le bouton d'appel du périphérique pendant plus de 5S pour\n\
            l'appariement du périphérique",
            
            "Localice el dispositivo de la puerta de la unidad, esperando la confirmación del dispositivo \n\
            Por favor, presione el botón de llamada del dispositivo durante más de 5 segundos \n\
            para emparejar el dispositivo",

            "Das Gerät wurde gefunden, wartet auf Gerätebestätigung \n\
            Bitte halten Sie die Anruftaste des Geräts länger als 5s gedrückt, um das Gerät zu koppeln ",
            "تم العثور على الجهاز ، في انتظار تأكيد الجهاز\n\
            اضغط باستمرار على زر الاتصال من الجهاز لمدة 5 ثوان أو أكثر إلى مباراة الجهاز",
            "Dveřní jednotka nalezena, čeká na potvrzení zařízení\n\
            Pro spárování zařízení podržte tlačítko volání více než 5s",
            "מכשיר הדלת של היחידה נמצא, מחכה לאישור מכשיר \n\
            אנא לחץ על כפתור השיחה של המכשיר במשך יותר מ-5 שניות כדי לזוג את המכשיר",
            "A Unidade Exterior foi Encontrada e \n\
            Aguarda a Confirmação da Unidade Interior \n\
            Pressione Segurando o Botão de Falar por mais de 5 segundos para Emparelhar",
            "È stato trovato un dispositivo della porta dell'unità ed è in attesa di conferma dal dispositivo\n\
            si prega di premere a lungo il pulsante di chiamata sul dispositivo per più di 5 secondi\n\
            accoppiamento del dispositivo",
            "Znaleziono urządzenie domofonu, oczekuje na potwierdzenie urządzenia\n\
            Naciśnij i przytrzymaj przycisk wywołania urządzenia przez ponad 5 sekund, aby sparować urządzenie",
            "Βρέθηκε συσκευή θυροτηλεφώνου, περιμένει την επιβεβαίωση της συσκευής\n\
            Πατήστε και κρατήστε πατημένο το κουμπί κλήσης της συσκευής για περισσότερα από 5 δευτερόλεπτα για να γίνει ο ζευγάρωμα της συσκευής",
            "Birim kapı makinesi bulundu, cihaz onayı bekleniyor\n\
            Cihazı eşleştirmek için lütfen cihazın çağrı düğmesini 5 saniyeden fazla basılı tutun",
            "Deurapparaat gevonden, wacht op apparaatbevestiging\n\
            Houd de oproepknop van het apparaat langer dan 5 seconden ingedrukt om het apparaat te koppelen"
        },
        {
            "Pairing completed, exiting!",
            "配对完成,正在退出!",
            "Добавление устройства завершено!",
            "Appariement terminé, sortie!",
            "Emparejamiento completo, saliendo!",
            "Kopplung abgeschlossen, beendet!",
            "الانتهاء من الاقتران ، الخروج!",
            "Párování dokončeno, ukončeno!",
            "זוג מושלם, יוצא",
            "Emparelhamento Concluído, A Sair!",
            "Accoppiamento completato! Esci.",
            "Parowanie zakończone, wychodzenie!",
            "Ολοκληρώθηκε το ζευγάρωμα, έξοδος!",
            "Eşleştirme tamamlandı, çıkılıyor!",
            "Koppelen voltooid, afsluiten!"
        },
        {"Find outdoor unit", "查找户外机", "Добавление устройств", "Trouver des unités extérieures", "Encontrar una unidad exterior", "Außeneinheit suchen", "ابحث عن وحدة في الهواء الطلق", "Vahledání dveřní jednotky", "מצא יחידה בחוץ", "Localizar Unidade Exterior", "Trova unità esterna"," Znajdź jednostkę zewnętrzną","Εύρεση εξωτερικής μονάδας","Dış üniteyi bul","Buitenunit zoeken"},
        {"Wallpaper Settings", "壁纸设置", "Настройка обоев", "Paramètres du papier peint", "Configuración del Fondo de pantalla", "Hintergrundbildereinstellungen", "خلفيات", "Nastavení tapety", "הגדרות נייר הארנק", "Configurações Imagem de Fundo", "Impostazioni sfondo","Ustawienia tła wyświetlacza","Ρυθμίσεις ταπετσαρίας","Duvar kağıdı Ayarları","Achtergrondinstellingen"},
        //layout system
        {"Software version","软件版本","Версия программного обеспечения","Une version de logiciel","Versión del software","Softwareversion","اصدار البرنامج","Verze softwaru", "גירסת תוכנה","Versão do Software","Versione software","wersja oprogramowania","Έκδοση λογισμικού","Yazılım sürümü","Software versie"},
        {"Release date","发布日期","Дата релиза","Date de sortie","Fecha de lanzamiento","Veröffentlichungsdatum","تاريخ الاصدار","Datum vydání", "תאריך שחרור","Data de Lançamento","Data di rilascio","data wydania","Ημερομηνία κυκλοφορίας","Yayın tarihi","Uitgebrachtatum"},
        {"SD remain space","SD卡剩余空间","Емкость карты памяти","SD reste de l'espace","SD queda espacio","SD bleibt Platz","المساحة المتبقية من بطاقة التخزين","Zbývající místo na SD kartě", "נפח פנוי ב SD","Espaço Livre no Cartão SD","Spazio rimanente SD","Wolna przestrzeń karty SD","SD εναπομέιναν χώρος","SD uzay kaldı","SD resterende ruimte"},
        {"UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID"},
        //layout video
        {"\n\nDelete ?\n\n","\n\n删除 ?\n\n","\n\nУдалить ?\n\n","\n\nSupprimez ?\n\n","\n\nEliminar ?\n\n","\n\nLöschen ?\n\n","\n\n? حذف\n\n","\n\nVymazat ?\n\n", "\n\nמחיקה ?\n\n","\n\nApagar?\n\n","\n\nElimina ?\n\n","\n\nUsunąć ?\n\n","\n\nΔιαγραφή ?\n\n","\n\nSil ?\n\n","\n\nVerwijderen ?\n\n"},
        //layout connect wifi
        {
            "Please input wifi password",
            "请输入wifi密码", 
            "Пожалуйста, введите пароль Wi-Fi",
            "Veuillez saisir le mot de passe Wi-Fi", 
            "Por favor ingrese la contraseña wifi",
            "Bitte geben Sie das WLAN-Passwort ein", 
            "الرجاء ادخال كلمة مرور واي فاي", 
            "Zadejte heslo pro WiFi", 
            "נא הכנס סיסמת WIFI", 
            "Introduza Senha Wi-Fi Por Favor",
            "Inserire la password Wi-Fi",
            "Wprowadź hasło Wi-Fi",
            "Παρακαλώ εισάγετε τον κωδικό πρόσβασης Wi-Fi",
            "Lütfen WiFi şifresini girin",
            "Voer het WiFi-wachtwoord in"
        },
        //layout background
        {
            "Setting failed!!!\nThe file cannot be larger than 2M",
            "设置失败\n文件不能大于2M",
            "Ошибка настройки\nфайл не может быть больше 2 Mбайт",
            "La configuration a échoué\nle fichier ne peut pas dépasser 2M",\
            "¡Configuración fallida!!! El archivo no puede ser mayor de 2M",
            "Einstellung fehlgeschlagen!!!\nDie Datei kann nicht größer als 2M sein",
            "فشل الإعداد ! \n  الملف لا يمكن أن يكون أكبر من 1 متر",\
            "Nastavení selhalo!!!\nSoubor nemůže být větší než 2M",
            "התקנה נכשלה!!! \n קובצים לא יכולים להיות גדולים יותר מ2M",
            "A configuração falhou!!! \n Os ficheiros não podem ser maiores do que 2M",
            "Impostazione fallita!!! \nIl file non può essere più grande di 2M",
            "Ustawienie nie powiodło się!!! \n Plik nie może być większy niż 2M",
            "Η ρύθμιση απέτυχε!!! \n Το αρχείο δεν μπορεί να είναι μεγαλύτερο από 2M",
            "Ayar başarısız oldu!!! \n Dosya 2M'den büyük olamaz",
            "Instelling mislukt!!! \n Het bestand mag niet groter zijn dan 2M"
        },
        {"Setting","正在设置","Настройки","Mise en place","Configuración en curso",
            "Einstellung","الإعداد","Nastavení","מתקן","A Configurar","Impostazioni","Ustawianie","Ρύθμιση","Ayarlar","Instellingen"},
        {"Set as wallpaper", "设为壁纸", "Установить обои", "Définir comme papier peint", 
            "Como fondo de pantalla", "Als Hintergrundbild festlegen", "تعيين خلفية", "Nastavit jako tapetu", "קבע כנייר קרקע","Definir como papel de parede","Imposta come sfondo","Ustaw jako tapetę","Ορισμός ως ταπετσαρία","Duvar kağıdı olarak ayarla","Instellen als achtergrond"},
        {"Restore factory wallpaper", "恢复出厂壁纸", "Восстановление заводских обоев", "Restaurer le fond d'écran d'usine", "Restaurar el papel pintado de fábrica", "Werkshintergrund wiederherstellen", "استعادة خلفية المصنع", "Obnovení tapety z výroby", "תחזיר נייר קרן מפעל","Restaurar o papel de parede de fábrica","Ripristinare lo sfondo di fabbrica","Ustaw jako tło","Ορίστε ως ταπετσαρία","Duvar kağıdı olarak ayarlayın","Stel in als achtergrond"},
        //layout customize ring
        {"Setting failed!!!\nThe file cannot be larger than 1M","设置失败\n文件不能大于1M","Ошибка настройки\nфайл не может быть больше 1 Mбайт","La configuration a échoué\nle fichier ne peut pas dépasser 1M",\
         "¡Configuración fallida!!! El archivo no puede ser mayor de 1M","Einstellung fehlgeschlagen!!!\nDie Datei kann nicht größer als 1M sein","فشل الإعداد ! \n  الملف لا يمكن أن يكون أكبر من 1 متر",\
         "Nastavení selhalo!!!\nSoubor nemůže být větší než 1M","התקנה נכשלה!!! \n קובצים לא יכולים להיות גדולים יותר מ1M","A configuração falhou!!! \n Os ficheiros não podem ser maiores do que 1M","Impostazione fallita!!! \nIl file non può essere più grande di 1M","Błąd! \n Plik nie może być większy niż 2 MB","Η ρύθμιση απέτυχε!!! \n Το αρχείο δεν μπορεί να είναι μεγαλύτερο από 1M","Ayar başarısız oldu!!! \n Dosya 1M'den büyük olamaz","Instelling mislukt!!! \n Het bestand mag niet groter zijn dan 1M"},
        //layout add wifi
        {
            "Connecting! Please wait...",
            "连接中!请等待...",
            "Подключение! Пожалуйста подождите…",
            "De liaison! S'il vous plaît, attendez...",
            "Conectando! Espere por favor...",
            "Verbinden! Warten Sie mal...",
            "جاري التوصيل يرجى الانتظار...",
            "Spojuji! Prosím čekéjte...",
            "מתחבר נא להמתין",
            "A Conectar! Por Favor, Aguarde",
            "In connessione, attendere!",
            "Łączenie! Proszę czekać...",
            "Σύνδεση! Παρακαλώ περιμένετε...",
            "Bağlanıyor! Lütfen bekleyin...",
            "Verbinden! Even geduld aub..."
        },
        {
            "Connection successful!",
            "连接成功!",
            "Подключение успешно!",
            "Connexion réussie!",
            "Conexión exitosa!",
            "Verbindung erfolgreich!",
            "!تم التوصيل بنجاح",
            "Spojeno úspěšně!",
            "חיבור בוצע בהצלחה",
            "Conecção Bem Sucedida",
            "Connessione OK",
            "Połączenie zakończone sukcesem!",
            "Η σύνδεση ήταν επιτυχής!",
            "Bağlantı başarılı!",
            "Verbinding geslaagd!"
        },
        {
            "Connection fail!",
            "连接失败!",
            "Ошибка подключения!",
            "Échec de la connexion!",
            "Conexión fallida!",
            "Verbindung fehlgeschlagen!",
            "!فشل الاتصال",
            "Chyba spojení!",
            "חיבור נכשל",
            "Conecção Falhada",
            "Connessione fallita",
            "Połączenie nieudane!",
            "Η σύνδεση απέτυχε!",
            "Bağlantı başarısız!",
            "Verbinding mislukt!"
        },
        {
            "Wifi name is empty!",
            "Wifi名为空!",
            "Пустое имя Wi-Fi !",
            "Le nom du Wi-Fi est vide!",
            "El nombre de wifi está vacío!",
            "WLAN-Name ist leer!",
            "!فشل الاتصال",
            "Název Wifi je prázdný!",
            "שם רשת WIFI ריק",
            "O Nome da Rede Wi-Fi Está Vazio",
            "Nome Wifi e vuoto",
            "Nazwa Wi-Fi jest pusta!",
            "Το όνομα Wi-Fi είναι κενό!",
            "WiFi adı boş!",
            "WiFi-naam is leeg!"
        },
        {
            "Wifi password is too short!",
            "Wifi密码太短!",
            "Пароль Wi-Fi слишком короткий!",
            "Le mot de passe Wifi est trop tiré!",
            "La contraseña de wifi está demasiado disparada!",
            "WLAN-Passwort ist zu kurz!",
            "!كلمة السر قصيرة جدا",
            "Heslo Wifi je příliš krátké!",
            "סיסמת WIFI קצרה מידי",
            "A Senha da Rede Wi-Fi é Muito Curta",
            "La password Wi-Fi è breve",
            "Hasło Wi-Fi jest zbyt krótkie!",
            "Ο κωδικός Wi-Fi είναι πολύ σύντομος!",
            "WiFi şifresi çok kısa!",
            "WiFi-wachtwoord is te kort!"
        },
        {
            "Not find!",
            "未找到!",
            "Не найти!",
            "Pas trouvé",
            "No encontrar!",
            "Nicht finden!",
            "!غير موجود",
            "Nenalezeno!",
            "לא נמצאה רשת",
            "Nenhuma Rede foi Encontrada",
            "Non trovato",
            "Nie znaleziono!",
            "Δεν βρέθηκε!",
            "Bulunamadı!",
            "Niet gevonden!"
        },
        {
            "Please input wifi name",
            "请输入wifi名称",
            "Пожалуйста, введите имя Wi-Fi",
            "Veuillez saisir le nom du Wi-Fi", 
            "Por favor ingrese el nombre wifi", 
            "Bitte geben Sie den WLAN-Namen ein",
            "الرجاء ادخال اسم واي فاي", 
            "Zadejte název wifi", 
            "נא הכנס שם רשת WIFI",
            "Introduza Nome da Rede Wi-Fi",
            "Inserire il nome Wi-Fi",
            "Wprowadź nazwę Wi-Fi",
            "Παρακαλώ εισάγετε το όνομα Wi-Fi",
            "Lütfen WiFi adını girin",
            "Voer de WiFi-naam in"
        },
        {
            "Please input wifi password",
            "请输入wifi密码", "Пожалуйста, введите пароль Wi-Fi",
            "Veuillez saisir le mot de passe Wi-Fi",
            "Por favor ingrese la contraseña wifi", 
            "Bitte geben Sie das WLAN-Passwort ein",
            "يرجى ادخال كلمة مرور واي فاي",
            "Zadejte heslo pro wifi",
            "נא הכנס סיסמת WIFI",
            "Introduza Senha Wi-Fi Por Favor",
            "Inserire la password Wi-Fi",
            "Wprowadź hasło Wi-Fi",
            "Παρακαλώ εισάγετε τον κωδικό πρόσβασης Wi-Fi",
            "Lütfen WiFi şifresini girin",
            "Voer het WiFi-wachtwoord in"
        },
        {"Wifi name:", "Wifi名称:", "Имя Wi-Fi:", "   Nom Wi-Fi:", "Nombre wifi:", "WLAN-Name:", ":اسم شبكة واي فاي", "Název WiFi:", "שם רשת WIFI ","Nome da Rede Wi-Fi","Nome Wi-Fi"," Nazwa Wi-Fi","Όνομα Wi-Fi","WiFi Adı","WiFi-naam"},
        {" Password:", "Wifi密码:", "   Пароль:", "Mot de passe:", "      Clave:", " Passwort:", ":كلمة السر        ", "Heslo WiFi:", "סיסמה","Senha","Password","Hasło:","Κωδικός πρόσβασης:","WiFi Şifresi:","Wachtwoord:"},
        //layout network common
        {"Outdoor Upgrade over!!!","室外机升级完成!!!","Обновление завершено!!!","Mise à niveau extérieure sur!!!","Actualización al aire libre!!!","Outdoor-Upgrade vorbei!!!","!!!الانتهاء من ترقية القطعة الخارجية ","Aktualizace je dokončena","שדרוג יחידה חיצונית הסתיים","Actualização da Unidade Exterior Terminada","Upgrade all'aperto terminato"," Aktualizacja pomyślna!!!","Αναβάθμιση εξωτερικού χώρου!!","Dışarıdaki Güncelleme bitti!","Buitenpost update gedaan!!!"},



};

static  char *lang_week_str[language_total][7]= {	
            {"Monday",    "Tuesday",    "Wednesday",	"Thursday",	 "Friday",   "Saturday",     "Sunday"},
            {"周一",	  "周二",  "周三",  "周四",  "周五",  "周六",  "周天"},
            {"Понедельник",	  "Вторник", "Среда",	"Четверг", 	 "Пятница",	  "Суббота",	 "Воскресенье"},	
            {"Lundi",	  "Mardi",    "Mercredi",	"Jeudi",	 "Vendredi",	  "Samedi",    "Dimanche"},
            {"Lunes",    "Martes",    "Miércoles",	"Jueves",	 "Viernes",    "Sábado",    "Domingo"},           
            {"Montag",	  "Dienstag",    "Mittwoch",	"Donnerstag",	 "Freitag",	  "Samstag",    "Sonntag"},
            {"اثنين", "ثلاثاء", "اربعاء", "خميس",  "جمعة",  "سبت",   "احد"},
            {"Pondělí" ,"Úterý", "Středa", "Čtvrtek", "Pátek", "Sobota", "Neděle"},
            {"שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת", "ראשון"}, 
            {"Seg ","Ter","Qua","Qui","Sex","Sáb","Dom"},
            {"Lunedi ","Martedi","Mercoledi","Giovedi","Venerdi","Sabato","Domenica"},
            {"Poniedziałek ","Wtorek","Środa","Czwartek","Piątek","Sobota","Niedziela"},
            {"Δευτέρα ","Τρίτη","Τετάρτη","Πέμπτη","Παρασκευή","Σάββατο","Κυριακή"},
            {"Pazartesi ","Salı","Çarşamba","Perşembe","Cuma","Cumartesi","Pazar"},
            {"Maandag ","Dinsdag","Woensdag","Donderdag","Vrijdag","Zaterdag","Zondag"},
        };

static  char *lang_mon_str[language_total][14]= { 
            {"January",    "February",  "March",  "April",  "May",  "June",   "July",    "August",  "September",   "October",  		"November",  	"December"},
            {"1月",	       "2月",	    "3月",    "4月",    "5月",  "6月",    "7月",     "8月", 		  "9月",         "10月",	    		"11月",	     	"12月"},
            {"Январь",     "Февраль",   "Март",   "Апрель", "Май",  "Июль",   "Июль",    "Август",  "Сентябрь",    "Октябрь",  		"Ноябрь",    	"Декабрь"},
            {"Janvier",    "Février",   "Mars",   "Avril",  "Mai",  "Juin",   "Juillet", "Août",    "Septembre",   "Octobre",  		"Novembre",  	"Décembre "},
            {"Enero",      "Febrero",   "Marzo",  "Abril",  "Mayo", "Junio",  "Julio",   "Agosto",  "Septiembre",  "Octubre",  		"Noviembre",	"Diciembre"},
            {"Januar",     "Februar",   "März",   "April",  "Mai",  "Juni",   "Juli",    "August",  "September",   "Oktober",  		"November",  	"Dezember"},
            {"يناير", "فبراير","مارس","أبريل","مايو","يونيو","يوليو","أغسطس","سبتمبر","اكتوبر","نوفمبر","ديسمبر"},
            {"Leden","Únor","Březen","Duben","Květen","Červen","Červenec","Srpen","Září","Říjen","Listopad","Prosinec"},
            {"בינואר","פברואר","מרץ","אפריל","מאי","יוני","יולי","אוגוסט","בספטמבר","אוקטובר","נובמבר","דצמבר"},
            {"Janeiro","Fevereiro","Março","Abril","Maio","Junho","Julho","Agosto","Setembro","Outubro","Novembro","Dezembro"},
            {"Gennaio","Febbraio","Marzo","Aprile","Maggio","Giugno","Luglio","Agosto","Settembre","Ottobre","Novembre","Dicembre"},
            {"Styczeń","Luty","Marzec","Kwiecień","Maj","Czerwiec","Lipiec","Sierpień","Wrzesień","Październik","Listopad","Grudzień"},
            {"Ιανουάριος","Φεβρουάριος","Μάρτιος","Απρίλιος","Μάιος","Ιούνιος","Ιούλιος","Αύγουστος","Σεπτέμβριος","Οκτώβριος","Νοέμβριος","Δεκέμβριος"},
            {"Ocak","Şubat","Mart","Nisan","Mayıs","Haziran","Temmuz","Ağustos","Eylül","Ekim","Kasım","Aralık"},
            {"Januari","Februari","Maart","April","Mei","Juni","Juli","Augustus","September","Oktober","November","December"},
        };

static  char * lang_btns_str[language_total][3] = {
        {"Cancle", "Confirm", ""},
        {"取消","确认",""},
        {"Отмена","Подтвердить",""},
        {"Annuler","Confirmer",""},
        {"Cancelar","Confirmar",""},
        {"Absagen","Bestätigen Sie",""},
        {"الغاء","تاكيد",""},
        {"Zrušit","Potvrdit",""},
        {"ביטול", "אישור",""},
        {"Cancelar","Confirmar", ""},
        {"Annulla","Conferma",""},
        {"Anuluj","Potwierdź",""},
        {"Ακύρωση","Επιβεβαίωση",""},
        {"Iptal et","Onaylayın",""},
        {"Annuleren","Bevestigen",""},
        
    };

static const char *lang_btnmatrix7_str[language_total][14] = {
                {"Ring 7",""},
                {"铃声 7", ""},
                {"Звонок 7",""},
                {"Sonnerie 7",""},
                {
                    "Anillo 7",""
                },
                {"Ring 7",""},
                {"رنين 7",""},
                {"Zvonění 7",""},
                {"7",""},
                {"Toque 7",""},
                {"Suoneria 7",""},
                {"Dźwięk 7",""},
                {"Κουδούνισμα 7",""},
                {"Yüzük tonu 7",""},
                {"Ring 7",""}

            };

static const char *lang_btnmatrix16_str[language_total][14] = {
            {"Ring 1", "\n", "Ring 2",  "\n","Ring 3", "\n", "Ring 4",  "\n","Ring 5", "\n","Ring 6",""},
            {"铃声 1", "\n", "铃声 2",  "\n","铃声 3", "\n", "铃声 4",  "\n","铃声 5", "\n","铃声 6",""},
		    {"Звонок 1", "\n", "Звонок 2", "\n", "Звонок 3", "\n", "Звонок 4", "\n", "Звонок 5", "\n","Звонок 6",""},
            {"Sonnerie 1","\n", "Sonnerie 2","\n", "Sonnerie 3","\n", "Sonnerie 4","\n", "Sonnerie 5","\n", "Sonnerie 6",""},
            {
                "Anillo 1","\n",
                "Anillo 2","\n",
                "Anillo 3","\n",
                "Anillo 4","\n",
                "Anillo 5","\n",
                "Anillo 6",""
            },
            {"Ring 1","\n", "Ring 2","\n", "Ring 3","\n", "Ring 4","\n", "Ring 5","\n", "Ring 6",""},
            {"رنين 1","\n", "رنين 2", "\n","رنين 3","\n", "رنين 4", "\n","رنين 5","\n", "رنين 6",""},
            {"Zvonění 1","\n", "Zvonění 2","\n", "Zvonění 3","\n", "Zvonění 4","\n", "Zvonění 5","\n", "Zvonění 6",""},
            {"1","\n", "2","\n", "3","\n", "4","\n", "5", "\n","6",""},
            {"Toque 1", "\n", "Toque 2",  "\n","Toque 3", "\n", "Toque 4",  "\n","Toque 5", "\n","Toque 6",""},
            {"Suoneria 1", "\n", "Suoneria 2",  "\n","Suoneria 3", "\n", "Suoneria 4",  "\n","Suoneria 5", "\n","Suoneria 6",""},
            {"Dźwięk 1", "\n", "Dźwięk 2",  "\n","Dźwięk 3", "\n", "Dźwięk 4",  "\n","Dźwięk 5", "\n","Dźwięk 6",""},
            {"Κουδούνισμα 1", "\n", "Κουδούνισμα 2",  "\n","Κουδούνισμα 3", "\n", "Κουδούνισμα 4",  "\n","Κουδούνισμα 5", "\n","Κουδούνισμα 6",""},
            {"Yüzük tonu 1", "\n", "Yüzük tonu 2",  "\n","Yüzük tonu 3", "\n", "Yüzük tonu 4",  "\n","Yüzük tonu 5", "\n","Yüzük tonu 6",""},
            {"Ring 1", "\n", "Ring 2",  "\n","Ring 3", "\n", "Ring 4",  "\n","Ring 5", "\n","Ring 6",""}
        };
    
static const char *lang_screenselct_str[language_total][4] = {
		{"Screen OFF", "\n", "Clock Display", ""},
		{"关闭屏幕", "\n", "时钟屏保", ""},
		{"Экран Выкл", "\n", "Отображение часов", ""},
		{"Écran éteint", "\n", "Horloge Affichage", ""},
		{"Pantalla apagada", "\n", "Pantalla de reloj", ""},
		{"Bildschirm AUS", "\n", "Uhranzeige", ""},
		{"قفل الشاسة", "\n", "عرض الساعة", ""},
		{"Vypnout obrazovku", "\n", "Zobrazit hodiny", ""},
		{"כיבוי מסך", "\n", "תצוגת שעון", ""},
		{"Écran Desligado", "\n", "Exibir Relógio", ""},
		{"Schermo spento", "\n", "Orologio", ""},
        {"Ekran wygaszony", "\n", "Zegar", ""},
        {"Απενεργοποίηση οθόνης", "\n", "Οθόνη ρολογιού", ""},
        {"Ekran Çıktı", "\n", "Saat Görüntüsü", ""},
        {"Scherm uit", "\n", "Klok display", ""}
        };

static const char *lang_formatselect_str[language_total][4] = {
			{"Format device", "\n", "Format SD card", ""},
			{"设备格式化", "\n", "SD卡格式化", ""},
			{"Форматирование устройства", "\n", "Формат SD карты", ""},
			{"Formater l'appareil", "\n", "Formater la carte SD", ""},
			{"Formatear dispositivo", "\n", "Formatear tarjeta SD", ""},
			{"Gerät formatieren", "\n", "SD-Karte formatieren", ""},
			{"تهيئة التجهار", "\n", "تهيئة بطاقة الذاكرة", ""},
			{"Smazat zařízení", "\n", "Naformátujte SD kartu", ""},
			{"אתחול מסך", "\n", "אתחול כרטיס SD", ""},
			{"Formatar Dispositivo", "\n", "Formatar Cartão SD", ""},
			{"Reset dispositivo", "\n", "Formatta la scheda SD", ""},
			{"Reset pamięci wewnętrznej", "\n", "Formatowanie karty SD", ""},
			{"Μορφοποίηση συσκευής", "\n", "Μορφοποίηση κάρτας SD", ""},
            {"Format aygıtı", "\n", "SD kart formatlaması", ""},
            {"Apparaat formatteren", "\n", "SD-kaart formatteren", ""}



		};

static const char *lang_deviceidselect_str[language_total][14] = {{"Device ID1",  	"\n", "Device ID2",  "\n", "Device ID3",  "\n", "Device ID4","\n", "Device ID5","\n", "Device ID6",  ""},
												{ "设备号 1",  	  	"\n", "设备号 2",     "\n", "设备号 3",     "\n", "设备号 4",  "\n", "设备号 5","\n", "设备号 6",   ""},
												{ "прылады: 1",      "\n", "прылады: 2", "\n", "прылады: 3",   "\n", "прылады: 4", "\n", "прылады: 5", "\n", "прылады: 6",""},
												{ "Moniteur 1",   "\n", "Moniteur 2",   "\n", "Moniteur 3",   "\n", "Moniteur 4", "\n", "Moniteur 5",   "\n", "Moniteur 6",  ""},
												{ "Equipo 1", "\n", "Equipo 2", "\n", "Equipo 3", "\n", "Equipo 4", "\n", "Equipo 5", "\n", "Equipo 6",""},
 												{ "Ausrüstung 1",	  "\n", "Ausrüstung 2",     "\n", "Ausrüstung 3",     "\n", "Ausrüstung 4", "\n", "Ausrüstung 5", "\n", "Ausrüstung 6", ""},
 												{ "1 معدات", "\n", "2 معدات",  "\n",  "3 معدات","\n", "4 معدات",  "\n",  "5 معدات","\n", "6 معدات",  "" },
 												{ "ID1 zařízení",   "\n", "ID2 zařízení",  "\n", "ID3 zařízení",  "\n", "ID4 zařízení",  "\n", "ID5 zařízení",  "\n", "ID6 zařízení", ""},
												{ "ID1 מכשיר",   "\n", "ID2  מכשיר",  "\n", "ID3  מכשיר",  "\n", "ID4  מכשיר",  "\n", "ID5  מכשיר",  "\n", "ID6  מכשיר", ""},
												{"ID1 do Dispositivo","\n","ID2 do Dispositivo","\n","ID3 do Dispositivo","\n","ID4 do Dispositivo","\n","ID5 do Dispositivo","\n","ID6 do Dispositivo",""},
												{"Ripetere ID1","\n","Ripetere ID2","\n","Ripetere ID3","\n","Ripetere ID4","\n","Ripetere ID5","\n","Ripetere ID6",""},
												{"Adres monitora1","\n","Adres monitora2","\n","Adres monitora3","\n","Adres monitora4","\n","Adres monitora5","\n","Adres monitora6",""},
												{" ID Συσκευής1","\n"," ID Συσκευής2","\n"," ID Συσκευής3","\n"," ID Συσκευής4","\n"," ID Συσκευής5","\n"," ID Συσκευής6",""},
                                                {"Aygıt ID1","\n","Aygıt ID2","\n","Aygıt ID3","\n","Aygıt ID4","\n","Aygıt ID5","\n","Aygıt ID6",""},
                                                {"Apparaat ID1","\n","Apparaat ID2","\n","Apparaat ID3","\n","Apparaat ID4","\n","Apparaat ID5","\n","Apparaat ID6",""}
        };


static const char *lang_brand_str[language_total][20] = {{"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//其它
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Другой
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Autres
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Otros
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Andere
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//اخرى
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Jiné
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//אחר
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Outros
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Altro
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Inne
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},//Άλλο
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""},
                                        {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""}//Anders
};


static const char *lang_stream_str[language_total][4] = {
			{"Main stream", "\n", "Subcode stream", ""},
			{"主码流", "\n", "子码流", ""},
			{"Основной поток", "\n", "Подкодовый поток", ""},
			{"Flux de code principal", "\n", "Sous-flux de code", ""},
			{"Flujo de código principal", "\n", "Flujo de código hijo", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Główny strumień kodu", "\n", "Strumień podkodu", ""},
			{"Main stream", "\n", "Subcode stream", ""},
			{"Main stream", "\n", "Subcode stream", ""},
            {"Main stream", "\n", "Subcode stream", ""},



		};

//获取通用字符串
 char *str_get(layout_lang_id id)
{
    // if(!is_lan_xls_init_ok_get())
    // {
    //     printf("Err: lan not init %d\n", id);
    // }else{
    //    printf("%s!!!!!!!!!!!!!!!!!内容\n",lang_xls_str_get(id+1,user_data_get()->user_language+1));

    // }
	return lang_xls_str_get(id+1,user_data_get()->user_language+1);
}

//获取月份字符串
char *mon_str_get(int mon){
    return lang_xls_str_get(mon+MONTH_LANG_JANUARY_ID+1,user_data_get()->user_language+1);
}
//获取星期字符串
char *week_str_get(int week){
    // printf("！！！！！！！！！！！！！%s",lang_xls_str_get(week+WEEK_LANG_MONDAY_ID+1,user_data_get()->user_language+1));
    return lang_xls_str_get(week+WEEK_LANG_MONDAY_ID+1,user_data_get()->user_language+1);
    // return lang_week_str[user_data_get()->user_language][week];
}

//获取消息框取消确认字符串
const void*btns_str_get(){
    return lang_btns_str[user_data_get()->user_language];
};

//获取铃声选择1-6字符串
const char **btnmatrix16_str_get(){

    return lang_btnmatrix16_str[user_data_get()->user_language];
};

//获取铃声选择7字符串
const char **btnmatrix7_str_get(){
 
    return lang_btnmatrix7_str[user_data_get()->user_language];
};

//获取屏保选择字符串
const char **screenselct_str_get(){

    return lang_screenselct_str[user_data_get()->user_language];
};

//获取格式化选择字符串
const char **formatselct_str_get(){
 
    return lang_formatselect_str[user_data_get()->user_language];
};

//获取设备id选择字符串
const char **deviceidselect_str_get(){
    return lang_deviceidselect_str[user_data_get()->user_language];
};

//获取cctv品牌选择字符串
const char **brand_str_get(){
    return lang_brand_str[user_data_get()->user_language];
};

const char **brand_str_value_get(int value){
    // printf("\n+++++++++++value=>%d+++++++\n",value);
    // printf("\n+++++++++++value_str=>%s+++++++\n",lang_brand_str[user_data_get()->user_language][2*value]);
    return lang_brand_str[user_data_get()->user_language][2*value];
};

//获取码流选择字符串
const char **stream_str_get(){
    return lang_stream_str[user_data_get()->user_language];
};
