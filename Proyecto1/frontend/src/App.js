import React, { useState } from 'react';
import './App.css';

function App() {
  const [comandos, setComandos]= useState('');
  const [salida, setSalida]= useState('');
  const [cargando, setCargando]= useState(false);

  const ejecutarComandos=async ()=> {
    if (!comandos.trim()){
      setSalida('No hay comandos para ejecutar.');
      return;
    }
    setCargando(true);
    setSalida('');
    try{
      const respuesta =await fetch('http://localhost:8080/execute', {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain' },
        body: comandos,
      });
      const texto = await respuesta.text();
      setSalida(texto);
    } 
    catch (error) {
      setSalida(' Error de conexión con el servidor: ' + error.message);
    }
    setCargando(false);
  };

  const cargarArchivo =(evento)=>{
    const archivo= evento.target.files[0];
    if (!archivo) {
      return;
    }
    const lector= new FileReader();
    lector.onload= (e) => {
      setComandos(e.target.result);
    };
    lector.readAsText(archivo);
  };

  const limpiar=() => {
    setComandos('');
    setSalida('');
  };

  return(
    <div className="App">
      <header className="App-header">
        <h1>ExtreamFS-Simulador de Discos MIA</h1>
        <p>Proyecto 1- C++ Disk</p>
      </header>

      <main className="contenedor">
        <section className="panel">
          <h2>Área de Comandos</h2>
          <textarea
            className="area-comandos"
            value={comandos}
            onChange={(e)=> setComandos(e.target.value)}
            placeholder={`Escribe aquí tus comandos, uno por línea.`}
            rows={15}
          />
          <div className="botones">
            <label className="boton-archivo">
               Cargar Script (.smia)
              <input
                type="file"
                accept=".smia,.txt"
                onChange={cargarArchivo}
                style={{ display: 'none' }}
              />
            </label>
            <button onClick={ejecutarComandos} disabled={cargando}>
              {cargando ? ' Ejecutando...' : '▶ Ejecutar'}
            </button>
            <button onClick={limpiar} className="boton-limpiar">
               Limpiar
            </button>
          </div>
        </section>

        <section className="panel">
          <h2> Área de Salida</h2>
          <pre className="area-salida">{salida || 'Aquí aparecerá la salida de los comandos...'}</pre>
        </section>
      </main>
    </div>
  );
}
export default App;