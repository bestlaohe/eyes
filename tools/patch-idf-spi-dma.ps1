# Patches ESP-IDF 5.1.x driver for spicommon_bus_enable_dma (required for TFT_eSPI DMA on ESP32-S3).
# Set IDF_PATH or pass -IdfPath C:\Users\...\esp\v5.1.4\esp-idf

param(
    [string]$IdfPath = $env:IDF_PATH
)

if (-not $IdfPath) {
    $IdfPath = "C:\Users\Administrator\esp\v5.1.4\esp-idf"
}

$common = Join-Path $IdfPath "components\driver\spi\gpspi\spi_common.c"
$header = Join-Path $IdfPath "components\driver\include\esp_private\spi_common_internal.h"

if (-not (Test-Path $common)) {
    Write-Error "spi_common.c not found at $common"
    exit 1
}

$hc = Get-Content $header -Raw
if ($hc -notmatch "spicommon_bus_enable_dma") {
    $hc = $hc -replace "(esp_err_t spicommon_dma_chan_alloc[^\r\n]+;)\r?\n\r?\n(/\*\*[\r\n]+\* @brief Free DMA for SPI)",
        "`$1`r`n`r`n/**`r`n * @brief Enable DMA on a bus already initialized with SPI_DMA_DISABLED`r`n */`r`nesp_err_t spicommon_bus_enable_dma(spi_host_device_t host_id, spi_dma_chan_t dma_chan);`r`n`r`n`$2"
    Set-Content -Path $header -Value $hc -NoNewline
    Write-Host "Patched $header"
} else {
    Write-Host "Already patched: $header"
}

$cc = Get-Content $common -Raw
if ($cc -notmatch "spicommon_bus_enable_dma") {
    $fn = @'

esp_err_t spicommon_bus_enable_dma(spi_host_device_t host_id, spi_dma_chan_t dma_chan)
{
    SPI_CHECK(is_valid_host(host_id), "invalid host_id", ESP_ERR_INVALID_ARG);
    SPI_CHECK(bus_ctx[host_id] != NULL, "SPI bus not initialized", ESP_ERR_INVALID_STATE);
#if SOC_GDMA_SUPPORTED
    SPI_CHECK(dma_chan == SPI_DMA_CH_AUTO, "invalid dma channel", ESP_ERR_INVALID_ARG);
#endif

    spicommon_bus_context_t *ctx = bus_ctx[host_id];
    spi_bus_attr_t *bus_attr = &ctx->bus_attr;
    if (bus_attr->dma_enabled) {
        return ESP_OK;
    }

    esp_err_t err = ESP_OK;
    uint32_t actual_tx_dma_chan = 0;
    uint32_t actual_rx_dma_chan = 0;

    if (dma_chan != SPI_DMA_DISABLED) {
        bus_attr->dma_enabled = 1;
        err = alloc_dma_chan(host_id, dma_chan, &actual_tx_dma_chan, &actual_rx_dma_chan);
        if (err != ESP_OK) {
            bus_attr->dma_enabled = 0;
            return err;
        }
        bus_attr->tx_dma_chan = actual_tx_dma_chan;
        bus_attr->rx_dma_chan = actual_rx_dma_chan;

        int dma_desc_ct = lldesc_get_required_num(bus_attr->bus_cfg.max_transfer_sz);
        if (dma_desc_ct == 0) {
            dma_desc_ct = 1;
        }
        bus_attr->max_transfer_sz = dma_desc_ct * LLDESC_MAX_NUM_PER_DESC;
        bus_attr->dmadesc_tx = heap_caps_malloc(sizeof(lldesc_t) * dma_desc_ct, MALLOC_CAP_DMA);
        bus_attr->dmadesc_rx = heap_caps_malloc(sizeof(lldesc_t) * dma_desc_ct, MALLOC_CAP_DMA);
        if (bus_attr->dmadesc_tx == NULL || bus_attr->dmadesc_rx == NULL) {
            dma_chan_free(host_id);
            bus_attr->dma_enabled = 0;
            return ESP_ERR_NO_MEM;
        }
        bus_attr->dma_desc_num = dma_desc_ct;
    }
    return ESP_OK;
}

'@
    $cc = $cc -replace "(const spi_bus_attr_t\* spi_bus_get_attr\(spi_host_device_t host_id\)\r?\n\{[\s\S]*?\r?\n\})\r?\n\r?\n(esp_err_t spi_bus_free)",
        "`$1`r`n`r`n$fn`r`n`$2"
    Set-Content -Path $common -Value $cc -NoNewline
    Write-Host "Patched $common"
} else {
    Write-Host "Already patched: $common"
}

Write-Host "Done. Rebuild the eyes project (driver component will recompile)."
