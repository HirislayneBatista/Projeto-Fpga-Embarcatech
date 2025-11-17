from ultralytics import YOLO 

# modelo pré treinado(sem quantização)
model = YOLO("best.pt")  

results = model.predict(
    source=0,      # 0 = webcam padrão
    show=True,     # mostra a janela com detecções
    conf=0.25,     # confiança mínima
    imgsz=640      # tamanho das imagens na entrada
)
